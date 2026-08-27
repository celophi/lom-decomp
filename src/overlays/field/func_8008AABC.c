#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0;

/**
 * @brief Argument block passed by pointer to func_800B5F60.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
} ArgB5F60;

extern Struct_D80105AE0 D_80105AE0[];
void func_800B5F60(ArgB5F60 *);

/**
 * @brief Builds a two-actor argument block and dispatches func_800B5F60.
 *
 * Copies the 0x14 field of actors @p a and @p b into a stack argument block,
 * marks it active (unk18 = 1), and passes it to func_800B5F60.
 */
void func_8008AABC(s32 a, s32 b)
{
    ArgB5F60 s;

    s.unk0 = D_80105AE0[a].unk14;
    s.unkC = D_80105AE0[b].unk14;
    s.unk18 = 1;
    func_800B5F60(&s);
}
