#include "common.h"

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Struct_D_800F2268;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
} Struct_D_800F2270;

extern Struct_D_800F2268 D_800F2268;
extern Struct_D_800F2270 D_800F2270;

/**
 * @brief Copy D_800F2270 unk0/unk2/unk4 into D_800F2268 and set D_800F2268.unk6 to 5.
 * @see decomp.me (100%) TODO
 */
void func_80067F28(void) {
    D_800F2268.unk6 = 5;
    D_800F2268.unk0 = (u16)D_800F2270.unk0;
    D_800F2268.unk2 = (u16)D_800F2270.unk2;
    D_800F2268.unk4 = (u16)D_800F2270.unk4;
}
