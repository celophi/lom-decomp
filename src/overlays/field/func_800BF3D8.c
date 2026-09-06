#include "common.h"

typedef struct
{
    u8 pad0[0x984];
    u16 unk984;
    u16 unk986;
    u16 unk988;
    u16 unk98A;
} RecFC0;

void func_800BF514(s32 arg0);
void func_800BF2F0(s32 arg0);

extern u8 *D_80123FC0;
extern u8 *D_80123FC4;

/**
 * @brief Process the active field effect indices stored in the shared state block.
 */
void func_800BF3D8(void)
{
    s32 i;

    func_800BF514(0);
    if (D_80123FC4[0x2D] < 0xA0)
    {
        func_800BF2F0(((RecFC0 *)(D_80123FC0 + (D_80123FC4[0x2D] << 3)))->unk98A);
    }
    i = 4;
    do
    {
        if (*(D_80123FC4 + i + 0x28) < 0xA0)
        {
            func_800BF2F0(((RecFC0 *)(D_80123FC0 + (*(D_80123FC4 + i + 0x28) << 3)))->unk988);
        }
        i -= 1;
    } while (i >= 3);
    if (D_80123FC4[0x2A] < 0xA0)
    {
        func_800BF2F0(((RecFC0 *)(D_80123FC0 + (D_80123FC4[0x2A] << 3)))->unk986);
    }
    if (D_80123FC4[0x29] < 0xA0)
    {
        func_800BF2F0(((RecFC0 *)(D_80123FC0 + (D_80123FC4[0x29] << 3)))->unk984);
    }
}
