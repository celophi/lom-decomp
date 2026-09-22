#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80058FF4(s32, s32, s32);
extern void func_8009A8D0(void);
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern s32 func_80099B50(s32);

/** @brief Set initial map coordinates, register the callback, and advance the sequence. */
void func_8009A854(void)
{
    func_8006D0F0(0x18, &D_800DCED8, &D_800DCEE4);
    func_80058FF4(0, D_800DCED8, D_800DCEE4);
    D_801398AC = 1;
    func_8006CBD8(&func_80099B50);
    D_801B2C4C += 1;
    func_8009A8D0();
}
