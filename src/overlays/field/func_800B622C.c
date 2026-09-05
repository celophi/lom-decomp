#include "common.h"

typedef struct B
{
    u16 unk0;
    u16 unk2;
    union
    {
        u32 unk4;
        struct
        {
            u16 lo;
            u16 unk6;
        } h;
    } u;
} B;

typedef struct Node
{
    u32 unk0;
    u32 unk4;
} Node;

typedef struct A
{
    u8 pad0[0x1C];
    u32 *unk1C;
    Node *unk20;
    B *unk24;
} A;

extern A *D_80123FB0;

/**
 * @brief Compute a tri-state status from the active record's flags.
 *
 * Reads the record at @c D_80123FB0->unk24. Returns 0 immediately when its
 * 0x4000 flag is set. Otherwise derives a parity @c flag from bit 9 of the
 * record's and the node's word fields, and a 2-bit selector @c sel from bits
 * 4-5 of @c *unk1C. For @c sel in {0,1} the result is -1 when @c flag is set
 * else 0; for @c sel in {2,3} the result is -1.
 *
 * @return -1, 0, or 0 per the selector/flag combination described above.
 * @note 79.93% match (gcc280_g0). Residue is a whole-function register
 *       rotation (target keeps @c D_80123FB0 in a0; this build uses v1) plus
 *       the coupled load-delay scheduling around it. The `do { } while (0)`
 *       wrapper is load-bearing (removing it drops the match to ~73%).
 */
s32 func_800B622C(void)
{
    s32 sel;
    s32 flag;
    s32 var_v0;
    B *temp_v1;
    Node *n20;
    u32 *pv;

    temp_v1 = D_80123FB0->unk24;
    if (temp_v1->u.h.unk6 & 0x4000)
    {
        return 0;
    }
    n20 = D_80123FB0->unk20;
    pv = D_80123FB0->unk1C;
    do
    {
        flag = ((((u32)temp_v1->u.unk4 >> 9) & 1) ^ (((u32)n20->unk4 >> 9) & 1)) & 0xFF;
    } while (0);
    sel = ((u32)*pv >> 4) & 3;
    var_v0 = sel < 2;
    if (sel != 1)
    {
        if (var_v0 != 0)
        {
            if (sel != 0)
            {
                return var_v0;
            }
            goto block_7;
        }
        if (sel < 4)
        {
            return -1;
        }
        return (sel < 4);
    }
block_7:
    if (flag != 0)
    {
        return -1;
    }
    return 0;
}
