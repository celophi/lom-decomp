#include "common.h"

typedef struct {
    u8 _pad15[0x15];
    u8 unk15;
    u8 _pad16[0x52 - 0x16];
    u16 unk52;
    u8 _pad54[0x5A - 0x54];
    u16 unk5A;
    u16 unk5C;
    u16 unk5E;
    u16 unk60;
    u16 unk62;
} Struct_8006429C;

void func_800640B4(Struct_8006429C *arg0);

/**
 * @brief Copy unk60/unk62 into unk5C/unk5E, zero unk15, store unk52 into unk5A,
 *        then call func_800640B4 with the same struct pointer.
 * @param arg0 Pointer to the target struct.
 * @see decomp.me (100%) TODO
 */
void func_8006429C(Struct_8006429C *arg0) {
    u16 temp_v0 = (u16)arg0->unk60;
    u16 temp_v1 = (u16)arg0->unk62;
    u16 temp_a1 = arg0->unk52;

    arg0->unk15 = 0;
    arg0->unk5C = temp_v0;
    arg0->unk5E = temp_v1;
    arg0->unk5A = temp_a1;
    func_800640B4(arg0);
}
