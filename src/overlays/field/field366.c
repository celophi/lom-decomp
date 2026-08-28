#include "common.h"

/** @brief Actor state struct fields touched when idling an animation. */
typedef struct Entry
{
    char pad00[0x21];
    u8 unk21;   /* 0x21 */
    char pad22[0x2A - 0x22];
    u16 unk2A;  /* 0x2A */
    char pad2C[0x2E - 0x2C];
    u16 unk2E;  /* 0x2E gate flag */
    char pad30[0x3A - 0x30];
    u8 unk3A;   /* 0x3A actor slot index */
} Entry;

/** @brief Actor slot record in D_80105AE0 (0x23C-byte stride). */
typedef struct Big
{
    char pad000[0x174];
    s32 unk174; /* 0x174 flag word */
    char pad178[0x23C - 0x178];
} Big;

extern Big D_80105AE0[];

/**
 * @brief Idle an actor's animation when its gate flag is clear.
 *
 * When @c unk2E is 0, stops the actor's sound, zeroes @c unk2A, runs
 * func_800952DC, clears the 0x1800 bits of the slot's @c unk174, bumps the low
 * bits of @c unk21, and notifies func_80096334.
 *
 * @param arg0 Actor state struct.
 * @see decomp.me (100%) TODO
 */
void func_80092550(Entry *arg0)
{
    if (arg0->unk2E == 0)
    {
        func_8006AA7C(arg0->unk3A);
        arg0->unk2A = 0;
        func_800952DC(arg0, 1);
        D_80105AE0[arg0->unk3A].unk174 = D_80105AE0[arg0->unk3A].unk174 & ~0x1800;
        arg0->unk21 = (u8)((arg0->unk21 & 0x80) + 2);
        func_80096334(arg0);
    }
}
