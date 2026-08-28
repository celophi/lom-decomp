#include "common.h"

typedef struct {
    u8 pad[0x44];
    u8 unk44;
} StructFC4;

extern u8 *D_80123FC4;
extern void func_800BF880(s32 arg0);

/**
 * @brief Drain a per-slot counter to zero across eight records.
 *
 * For each of the eight byte-strided records at @c D_80123FC4, repeatedly
 * dispatches the slot index to func_800BF880 and decrements the record's
 * @c unk44 counter until it reaches zero.
 *
 * @see decomp.me (100%) TODO
 */
void func_800BF800(void)
{
    s32 i;

    for (i = 0; i < 8; i++)
    {
        while (((StructFC4 *)(D_80123FC4 + i))->unk44 != 0)
        {
            func_800BF880(i);
            ((StructFC4 *)(D_80123FC4 + i))->unk44--;
        }
    }
}
