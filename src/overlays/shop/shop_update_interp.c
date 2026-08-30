#include "common.h"
extern s32 D_80145248;
extern s32 D_8014524C;
extern s32 D_80145CE0;
extern void func_80140D4C(void);
extern void func_80140838(void);
void func_80140798(void)
{
    s32 count;
    s32 current;
    s32 target;
    s32 delta;
    s32 *current_p;

    func_80140D4C();
    func_80140838();
    count = D_80145CE0;
    if (count != 0) {
        current_p = &D_80145248;
        target = D_8014524C;
        current = *current_p;
        delta = (target - current) / count;
        D_80145CE0 = count - 1;
        *current_p = current + delta;
    } else {
        D_80145248 = D_8014524C;
    }
}
