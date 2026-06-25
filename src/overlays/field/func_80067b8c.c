#include "common.h"

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
} Struct_D_800F2290;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Struct_D_800F2268;

extern Struct_D_800F2290 D_800F2290;
extern Struct_D_800F2268 D_800F2268;

/**
 * @brief Zero all fields of D_800F2290 and D_800F2268.
 * @see decomp.me (100%) TODO
 */
void func_80067B8C(void) {
    D_800F2290.unk0 = 0;
    D_800F2290.unk2 = 0;
    D_800F2290.unk4 = 0;
    D_800F2268.unk0 = 0;
    D_800F2268.unk2 = 0;
    D_800F2268.unk4 = 0;
    D_800F2268.unk6 = 0;
}
