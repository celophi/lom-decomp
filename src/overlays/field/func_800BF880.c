#include "common.h"

typedef struct
{
    u8 pad0[8];
    s32 unk8; /* 0x08 */
    u8 padC[0x58 - 0xC];
    s32 unk58; /* 0x58 */
} StructFC4;

extern StructFC4 *D_80123FC4;

/**
 * @brief Deducts a scaled resource cost from the shared pool for one record.
 *
 * The per-record entry lives at byte offset @p arg0 * 2 within the global
 * @c *D_80123FC4 block: a "cost" byte at 0xC (defaulting to 1 when zero) and a
 * "level" byte at 0xD. The cost is shifted left by the level; if the pool total
 * at @c unk8 can cover it and the level is still below 0xF, the shifted cost is
 * subtracted from the pool and the level is incremented.
 *
 * @param arg0 Record index; the entry byte offset is @p arg0 * 2.
 */
void func_800BF880(s32 arg0)
{
    StructFC4 *p;
    u8 *e;
    s32 amount;
    s32 count;

    amount = arg0 * 2;
    p = D_80123FC4;
    e = (u8 *)p + amount;

    amount = 1;
    if (e[0xC] != 0)
    {
        amount = e[0xC];
    }

    count = p->unk8;
    amount = amount << e[0xD];

    if ((count >= amount) && (e[0xD] < 0xF))
    {
        p->unk8 = count - amount;
        e[0xD] = e[0xD] + 1;
    }
}
