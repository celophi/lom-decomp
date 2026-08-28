#include "common.h"

/** @brief Actor slot record in the D_80105AE0 array (0x23C-byte stride). */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;   /* 0x0C */
    u8 padC[0x170 - 0x10];
    u8 unk170;  /* 0x170 linked-slot index */
    u8 pad171[3];
    s32 unk174; /* 0x174 */
    u32 unk178; /* 0x178 flag word */
    u8 pad17C[0x23C - 0x17C];
} Record;

/** @brief Caller struct; only the actor slot index at 0x3A is read. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;   /* 0x3A */
} ArgStruct;

extern Record D_80105AE0[];

/**
 * @brief Clear a pending flag and its linked slot's draw bit for an actor.
 *
 * When bit 1 of the actor slot's @c unk178 flag word is set, clears it and also
 * clears the 0x2000 draw bit of @c unkC on the slot referenced by @c unk170.
 *
 * @param arg0 Caller struct holding the actor slot index at @c unk3A.
 * @see decomp.me (100%) TODO
 */
void func_8008BC5C(ArgStruct *arg0)
{
    Record *base = D_80105AE0;
    Record *temp_a1;
    u32 temp_v1;
    Record *temp_v0;

    temp_a1 = &base[arg0->unk3A];
    temp_v1 = temp_a1->unk178;
    if ((temp_v1 >> 1) & 1)
    {
        temp_a1->unk178 = temp_v1 & ~2;
        temp_v0 = &base[base[arg0->unk3A].unk170];
        temp_v0->unkC = temp_v0->unkC & ~0x2000;
    }
}
