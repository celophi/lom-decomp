#include "common.h"

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

typedef struct {
    u32 attr;
    u32 unk4;
    void (*draw_handler)();
} CardaPacket;

extern s32 D_80122988;
extern s32 D_8012299C;
extern u16 D_8014B0D2;
extern u8 D_80165B70[];
extern CardaPacket D_80165F80;
extern s32 D_80165FEC;
extern s32 D_80165FF4;
extern s32 D_80165FFC;
extern s32 D_80165F38;
extern s32 D_80166078;
extern s32 D_801660A0;
extern s32 D_801660F8;
extern s32 D_801660FC;
extern s32 D_80166104;
extern void *D_801663A0;
extern s32 D_80166ADC;

s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
void func_800A3938();
void func_80067F28(void);
void func_80147C5C(void);
void func_8014A044(void);
void func_80149FEC(void);

s32 func_80146794(s32 prim, s32 *ot, s32 arg2, s32 arg3)
{
    CardaPacket *p;
    s32 result;
    s32 i;

    result = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0D2, 0x9A), 4, arg2, arg3, 2);

    if (D_80122988 & 0xA000) {
        D_80165FEC = 0xF1;
        D_801660A0 ^= 1;
        func_800A3938(0x7D, 0x80);
        return result;
    }

    if (D_80122988 & 0x40) {
        switch (D_80166078) {
        case 2:
            D_8012299C = 6;
            break;
        case 3:
            D_8012299C = 7;
            break;
        default:
            D_8012299C = 3;
            break;
        }
        func_800A3938(0x78, 0x80);
        func_80067F28();
        p = &D_80165F80;
        for (i = 0; i < 8; i++, p++) {
            if (p->attr & 7) {
                p->attr = (((p->attr & ~7) | 3) & ~0x78) | 0x40;
            }
        }
        return result;
    }

    if (D_80122988 & 0x220) {
        s32 slot;
        func_800A3938(0x7D, 0x80);
        slot = D_801660A0;
        D_801660F8 = 0;
        D_801663A0 = 0;
        D_80165FEC = 0xFF;
        D_80165FFC = 0;
        D_80165F38 = 0;
        D_80166104 = 0;
        D_80165FF4 = 0;
        D_801660FC = 0;
        *(volatile s32 *)&D_801660A0 = slot ^ 1;
        D_801660A0 = slot;
        func_80147C5C();
        func_8014A044();
        func_80149FEC();
        D_80166ADC = 0;
        D_80165FEC = 0xFF;
        D_801663A0 = D_80165B70;
    }

    return result;
}
