#include "common.h"

void field_set_fade_target_only();
void func_800A3938();

extern s32 D_8012269C;
extern s32 D_801227E0;

void func_800AB638(s32 arg0)
{
    D_8012269C = 6;
    field_set_fade_target_only(0xC0, 0x80, 0x80, 8);
    func_800A3938(0xC7, 0x80);
    D_801227E0 = arg0;
}
