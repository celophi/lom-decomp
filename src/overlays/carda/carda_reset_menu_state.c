#include "common.h"

extern s32 D_801660F8;
extern void *D_801663A0;
extern s32 D_80165FEC;
extern s32 D_80165FFC;
extern s32 D_80165F38;
extern s32 D_80166104;
extern s32 D_80165FF4;
extern s32 D_801660A0;
extern s32 D_801660FC;

void func_801410E4(void)
{
    D_801660F8 = 0;
    D_801663A0 = 0;
    D_80165FEC = 0xFF;
    D_80165FFC = 0;
    D_80165F38 = 0;
    D_80166104 = 0;
    D_80165FF4 = 0;
    D_801660FC = 0;
    D_801660A0 ^= 1;
    func_80147C5C();
    func_8014A044();
    func_80149FEC();
}
