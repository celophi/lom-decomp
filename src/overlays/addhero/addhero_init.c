#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

extern s32 D_8016093C;
extern s32 D_801609A4;
extern s32 D_801609A8;
extern s32 D_8016092C;
extern s32 D_80160920;
extern s32 D_80165200;
extern s32 D_80160934;
extern s32 D_801609B8;
extern s32 D_80164A40;
extern s32 D_801609C4;
extern s32 D_801609A0;
extern s32 D_80160930;

extern void func_801449F0(void);
extern void func_801458D0(void);
extern void func_80067F8C(void);
extern void func_8001990C(RECT *, s32, s32, s32);
extern void func_80146DE0(void);
extern void func_800AA02C(void);
extern void func_8014028C(void);

/** @see decomp.me (100%) */
void func_8014011C(s32 arg0, s32 arg1)
{
    RECT rect;

    D_8016093C = arg1;
    D_801609A4 = 0xFF;
    D_801609A8 = 0;
    func_801449F0();
    D_8016092C = 3;
    func_801458D0();
    D_80160920 = 0;
    func_80067F8C();
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
    func_8001990C(&rect, 0, 0, 0);
    func_80146DE0();
    D_80165200 = 0;
    D_80160934 = 0;
    D_801609B8 = 0;
    D_80164A40 = 0;
    D_801609C4 = 0;
    D_801609A0 = 0;
    func_800AA02C();
    func_8014028C();
    D_80160930 = arg0;
}
