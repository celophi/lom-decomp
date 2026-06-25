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
 * @brief Write arg0/arg1/arg2 into both D_800F2268 and D_800F2270 (unk0/unk2/unk4),
 *        and write arg3 into D_800F2268.unk6.
 * @param arg0 Value written to unk0 of both structs.
 * @param arg1 Value written to unk2 of both structs.
 * @param arg2 Value written to unk4 of both structs.
 * @param arg3 Value written to D_800F2268.unk6 only.
 * @see decomp.me (100%) TODO
 */
void func_80067EB4(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    D_800F2268.unk0 = arg0;
    D_800F2270.unk0 = arg0;
    D_800F2268.unk2 = arg1;
    D_800F2270.unk2 = arg1;
    D_800F2268.unk4 = arg2;
    D_800F2270.unk4 = arg2;
    D_800F2268.unk6 = arg3;
}
