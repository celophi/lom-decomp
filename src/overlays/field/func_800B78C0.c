#include "common.h"

typedef struct
{
    u8 unk0;
    u8 unk1;
} FieldB78C0Rec;

typedef struct
{
    char pad[0x1C];
    s32 *unk1C;
    s32 unk20;
    s32 unk24;
} FieldB78C0State;

extern void func_800B2B54(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern s32 func_800B4CE4(s32 a, s32 b);
extern FieldB78C0Rec D_800F0BC0;
extern FieldB78C0State *D_80123FB0;

void func_800B78C0(void)
{
    s32 var_s0;
    FieldB78C0Rec *new_var;
    FieldB78C0Rec *temp_v1;

    var_s0 = 0x50;
    if ((*D_80123FB0->unk1C & 0xF) != 2)
    {
        do
        {
            if (func_800B4CE4(D_80123FB0->unk20, var_s0) != 0)
            {
                new_var = &D_800F0BC0;
                temp_v1 = &new_var[var_s0 - 0x50];
                func_800B2B54(D_80123FB0->unk20, D_80123FB0->unk24, 0, var_s0 - 0x50,
                              (s32)temp_v1->unk0, temp_v1->unk1 * 0x10);
            }
            var_s0 += 1;
        } while (var_s0 < 0x60);
    }
}
