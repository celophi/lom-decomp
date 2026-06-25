#include "common.h"

typedef struct {
    u8 _pad[0x2C];
    s32 unk2C;
    s32 unk30;
} FieldState;

extern unsigned int D_801ED02C;

/**
 * @brief If D_801ED02C is zero, set it to 1 and write 0x100 to D_801ED030.
 * @see decomp.me (100%) TODO
 */
void func_8005B1EC(void) {
    volatile FieldState *s = (volatile FieldState *)0x801ED000;
    if (s->unk2C == 0) {
        s->unk2C = 1;
        s->unk30 = 0x100;
    }
}

/**
 * @brief Return non-zero if D_801ED02C is set.
 * @return 1 if D_801ED02C != 0, 0 otherwise.
 * @see decomp.me (100%) TODO
 */
s32 func_8005B218(void) {
    return D_801ED02C != 0;
}
