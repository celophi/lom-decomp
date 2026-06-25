#include "common.h"

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Struct_D_800F2268;

extern Struct_D_800F2268 D_800F2268;

/**
 * @brief Write four s16 values into the four fields of D_800F2268.
 * @param arg0 Value for unk0.
 * @param arg1 Value for unk2.
 * @param arg2 Value for unk4.
 * @param arg3 Value for unk6.
 * @see decomp.me (100%) TODO
 */
void func_80067F0C(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    D_800F2268.unk0 = arg0;
    D_800F2268.unk2 = arg1;
    D_800F2268.unk4 = arg2;
    D_800F2268.unk6 = arg3;
}
