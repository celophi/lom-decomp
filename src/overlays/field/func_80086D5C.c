#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;     /* 0xC */
    u8 pad10[0x170 - 0x10];
    u8 unk170;    /* 0x170 */
    u8 pad171[0x178 - 0x171];
    u32 unk178;   /* 0x178 */
    u8 pad17C[0x23C - 0x17C];
} Struct_D80105AE0;

extern Struct_D80105AE0 D_80105AE0[];

/**
 * @brief Clears a linked actor's 0x2000 flag from a record's actor index.
 *
 * Uses the 0x3A index byte of @p p to select an actor; if bit 1 of its 0x178
 * word is set, clears bit 13 (0x2000) of the 0xC flags word belonging to the
 * actor referenced by its 0x170 byte.
 */
void func_80086D5C(u8 *p)
{
    Struct_D80105AE0 *base = D_80105AE0;
    Struct_D80105AE0 *e = &base[p[0x3A]];

    if ((e->unk178 >> 1) & 1)
    {
        Struct_D80105AE0 *e2 = &base[e->unk170];
        e2->unkC &= ~0x2000;
    }
}
