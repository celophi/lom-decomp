#include "common.h"

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Struct_D_800F2268;

extern Struct_D_800F2268 D_800F2268;

/**
 * @brief Initialize D_800F2268 to the default color/intensity values (0xC0, 0xC0, 0xC0, 5).
 * @see decomp.me (100%) TODO
 */
void func_80067F8C(void) {
    D_800F2268.unk0 = 0xC0;
    D_800F2268.unk2 = 0xC0;
    D_800F2268.unk4 = 0xC0;
    D_800F2268.unk6 = 5;
}
