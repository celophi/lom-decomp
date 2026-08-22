#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

extern s32 D_80164A78;
extern s32 D_80164AD0;
extern s32 D_80164AD4;
extern s32 D_80164AE4;
extern s32 D_80164AE8;
extern s32 D_80164B70;
extern s32 D_80164B74;
extern s32 D_80164B78;
extern s32 D_80164B84;
extern s32 D_80164B8C;
extern s32 D_80164B90;

void func_8014011C(s32 arg0, s32 arg1)
{
    RECT rect;

    D_80164AE8 = arg1;
    D_80164B78 = 0xFF;
    D_80164B70 = 0;
    func_80144BC0();
    func_80145A40();
    D_80164AD0 = 0;
    func_80067F8C();
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
    func_8001990C(&rect, 0, 0, 0);
    func_80146F34();
    D_80164B90 = 0;
    D_80164AD4 = 0;
    D_80164B84 = 0;
    D_80164A78 = 0;
    D_80164B8C = 0;
    D_80164B74 = 0;
    func_800AA02C();
    func_8014027C();
    D_80164AE4 = arg0;
}