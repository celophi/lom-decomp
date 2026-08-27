#include "common.h"

typedef struct ActorB4934
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[5];
    u16 unkA;
} ActorB4934;

extern u16 D_800F0B58[];
extern u8 *D_80122B74;

/**
 * @brief Rebuild an actor's 16-bit status mask from its four sub-entries.
 *
 * Clears the actor's 0xA mask, then walks the four 0x40-byte records that begin
 * at offset 0x5F0 of the actor's per-index block (stride 0x250) inside the table
 * pointed to by @c D_80122B74. For each active record (byte 0 non-zero) it ORs in
 * the 16-bit flag looked up in @c D_800F0B58 by the record's 0x2E field.
 *
 * @param arg0 Actor whose 0x4 index selects the block and whose 0xA mask is set.
 * @note gcc280_g0, 100% match.
 */
void func_800B4934(ActorB4934 *arg0)
{
    s32 i;
    s32 off;
    u8 *base;
    u8 *rec;
    u16 *tbl;

    i = 0;
    tbl = D_800F0B58;
    arg0->unkA = 0;
    off = 0x50;
    base = D_80122B74 + (arg0->unk4 * 0x250 + 0x5F0);
    do
    {
        rec = base + off;
        if (*rec != 0)
        {
            arg0->unkA |= tbl[*(u16 *)(rec + 0x2E)];
        }
        i += 1;
        off += 0x40;
    } while (i < 4);
}
