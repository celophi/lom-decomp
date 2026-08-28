#include "common.h"

/** @brief Actor slot record in the D_80105AE0 array (0x23C-byte stride). */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;   /* 0x0C */
    u8 padC[0x170 - 0x10];
    u8 unk170;  /* 0x170 */
    u8 pad171[3];
    s32 unk174; /* 0x174 flag word */
    u32 unk178; /* 0x178 */
    u8 pad17C[0x23C - 0x17C];
} Record;

/** @brief Actor state struct operated on when arming an animation. */
typedef struct
{
    u8 pad0[0x1B];
    s8 unk1B;   /* 0x1B */
    u8 pad1C[0x20 - 0x1C];
    s8 unk20;   /* 0x20 */
    u8 unk21;   /* 0x21 */
    u8 pad22[0x24 - 0x22];
    s8 unk24;   /* 0x24 */
    u8 pad25[0x27 - 0x25];
    s8 unk27;   /* 0x27 */
    u8 pad28[0x2A - 0x28];
    s16 unk2A;  /* 0x2A */
    u8 pad2C[0x2E - 0x2C];
    s16 unk2E;  /* 0x2E */
    u8 pad30[0x3A - 0x30];
    u8 unk3A;   /* 0x3A actor slot index */
} ArgStruct;

extern Record D_80105AE0[];
void func_8006C3FC(ArgStruct *arg0);

/**
 * @brief Prime an actor's state fields and clear its slot animation bits.
 *
 * Writes the standard "start animation" state block (@c unk2A/2E/24/1B/27/21),
 * clears the 0x1800 bits of the actor slot's @c unk174, notifies func_8006C3FC,
 * and latches @p arg3 into @c unk20.
 *
 * @param arg0 Actor state struct.
 * @param arg1 Value stored into @c unk1B.
 * @param arg2 Low-bit contribution to @c unk21.
 * @param arg3 Value stored into @c unk20.
 * @see decomp.me (100%) TODO
 */
void func_8008BF88(ArgStruct *arg0, s8 arg1, s32 arg2, s8 arg3)
{
    Record *base = D_80105AE0;
    Record *temp_v0;

    arg0->unk2A = 0x8F;
    arg0->unk2E = 1;
    arg0->unk24 = 1;
    arg0->unk1B = arg1;
    arg0->unk27 = 0;
    arg0->unk21 = (arg0->unk21 & 0x80) + arg2;
    temp_v0 = &base[arg0->unk3A];
    temp_v0->unk174 = temp_v0->unk174 & ~0x1800;
    func_8006C3FC(arg0);
    arg0->unk20 = arg3;
}
