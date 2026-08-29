#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800A4744(void);
u8 func_800A4778(void);
void func_800B286C(s32 arg0, s32 arg1, s32 arg2);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

void func_800BA6BC(void)
{
    s32 temp_v0;
    SeqRec *temp_v1;

    temp_v0 = func_800A4744();
    if (temp_v0 < 0)
    {
        func_800B286C(0x80, 0, func_800A4778() & 0xFF);
        ((SeqRec *)D_80123FB8)->unk0 &= 0x7FFFFFFF;
        return;
    }
    func_800BD520(0, 0x7100, temp_v0);
    func_800B286C(0x80, 0, 0x10);
    temp_v1 = (SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2));
    temp_v1->unk8 += 1;
}
