#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 pad8[0x14];
    s32 unk1C;
    s32 unk20;
    u8 pad24[0x14];
    s32 unk38;
    s32 unk3C;
} FieldInitState;

extern FieldInitState D_80105880;
extern s32 D_80105790;
extern s32 D_80105870;
extern s32 D_80105878;
extern s32 D_8010D034;

void func_8008B724(void);
void func_8009A384(void);
void func_8009CA08(s32 arg0, s32 arg1);
s32 func_8009CA54(s32 arg0, s32 arg1, s32 arg2);

void func_80084240(void)
{
    func_8008B724();
    D_80105880.unk38 = 0;
    D_80105880.unk1C = 0;
    D_80105880.unk0 = 0;
    D_80105880.unk3C = 0;
    D_80105880.unk20 = 0;
    D_80105880.unk4 = 0;
    func_8009A384();
    func_8009CA08(D_8010D034, 0x20000);
    D_80105870 = func_8009CA54(D_8010D034, 0x1800, 4);
    D_80105790 = func_8009CA54(D_8010D034, 0xC00, 4);
    D_80105878 = func_8009CA54(D_8010D034, 0xC00, 4);
}
