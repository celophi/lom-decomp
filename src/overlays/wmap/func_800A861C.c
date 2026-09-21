#include "common.h"

extern void akao_cmd_f1(void);
extern s32 D_801B2E58;
extern s32 D_801B2E5C;

/** @brief Issue audio command F1, start a 60-tick delay, and advance the state. */
void func_800A861C(void)
{
    akao_cmd_f1();
    D_801B2E5C = 60;
    D_801B2E58 += 1;
}
