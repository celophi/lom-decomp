#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 pad8[0x23C - 8];
} Struct_D80105AE0;

extern u8 *D_80122B78;
extern s32 *D_80123FB0;

void func_800966F0();
void func_800B28E0(s32, s32, s32);
Struct_D80105AE0 *func_80087F0C(s32 arg0);

/**
 * @brief Mark the field state busy and refresh matching actor records.
 */
void func_800B4584(void)
{
    s32 i;
    s32 off;
    Struct_D80105AE0 *rec;
    s32 *p;

    p = D_80123FB0;
    *p |= 0x80000000;
    func_800966F0(0, p);
    func_800B28E0(0x80, 0xD, 3);

    for (i = 3; i < *(u16 *)(D_80122B78 + 0x400); i++)
    {
        off = i * 0x94;
        if (((*(u32 *)(D_80122B78 + off + 0x4C0)) & 0xF) == *(u8 *)(D_80122B78 + 0x403))
        {
            func_800B28E0((D_80122B78 + off)[0x430], 0xD, 3);
        }
    }

    for (i = 0; i < 3; i++)
    {
        rec = func_80087F0C(i);
        if (rec != (Struct_D80105AE0 *)-1)
        {
            rec->unk4 = rec->unk0;
        }
    }
}
