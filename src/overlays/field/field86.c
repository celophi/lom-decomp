#include "common.h"

typedef struct
{
    u8 unk0;
    u8 _pad1[3];
    u8* unk4;
} FieldTextMacro;

extern FieldTextMacro D_80122B80[];

void func_800B2844(s32 arg0, u8* arg1, u8 arg2)
{
    if (arg0 < 0x10)
    {
        D_80122B80[arg0].unk0 = arg2;
        D_80122B80[arg0].unk4 = arg1;
    }
}
