#include "common.h"
#include "vector.h"

/**
 * @brief Two-byte relative offset into the shared FIELD string table.
 */
typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

extern StructEC D_800EC3D8;
extern void *D_801227F8[];
extern s32 D_80122908;
extern u8 D_80122910[];

extern s32 func_800A88A0(s32 prim, s32 *ot, void *tex, s32 arg3, s32 x, s32 y, s32 z);
extern s32 func_800A8A78(s32 *ot, s32 prim, u32 val, s32 arg3, Vec2s *pos, s32 arg5);

/**
 * @brief Draw a FIELD text list and the nonzero amount beside each entry.
 * @param ot Ordering table receiving the generated primitives.
 * @param prim Current primitive-chain handle.
 * @param arg2 Horizontal origin adjustment, also used by the amount row position.
 * @param arg3 Vertical origin adjustment for the list text.
 * @return Updated primitive-chain handle after drawing the list.
 * @note Preserve the distinct origin adjustments used by text and amounts.
 * @note The unused local buffer preserves the observed 0xD8-byte frame.
 */
s32 func_800A7FB4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 i;
    u8 val;
    Vec2s pos;
    u8 pad[0x84];
    s32 row;
    s32 low;
    s32 offset;
    u8 *tex;

    i = 0;
    low = D_800EC3D8.unk0;
    offset = (D_800EC3D8.unk1 << 8) + (s32)((u8 *)&D_800EC3D8 - 0x14);
    tex = (u8 *)(low + offset);
    prim = func_800A88A0(prim, ot, tex, 4, 0x20 - arg2, -arg3, 0);
    if (D_80122908 > 0)
    {
        do
        {
            row = i * 0x10;
            prim = func_800A88A0(prim, ot, D_801227F8[i], 4, 0x10 - arg2, row - (arg3 - 0x10), 0);
            pos.x = 0xB0 - arg2;
            row -= arg2 - 0x10;
            pos.y = row;
            tex = &D_80122910[i];
            val = *tex;
            if (val != 0)
            {
                prim = func_800A8A78(ot, prim, val, 4, &pos, 1);
            }
            i += 1;
        } while (i < D_80122908);
    }
    return prim;
}
