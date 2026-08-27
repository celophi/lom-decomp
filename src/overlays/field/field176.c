#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
} UnkStruct_80122C0C;

/**
 * @brief Thin stack-frame wrapper around func_800C3A00 with a fixed arg.
 */
void func_800C6344(void)
{
    func_800C3A00(0x92BC);
}

extern UnkStruct_80122C0C D_80122C0C;

void func_800C6364(void)
{
    func_80087680(D_80122C0C.unk0, D_80122C0C.unk2, D_80122C0C.unk2, 0, 0, 0);
}
