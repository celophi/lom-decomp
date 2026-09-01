#include "common.h"

extern s16 D_80122C00[];
extern s16 D_80122C06[];
extern s16 D_80122C10;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Invalidates matching menu slots and preserves the remaining IDs.
 *
 * Loads three menu IDs from the active 0x40-byte layout record, then scans
 * the three halfword slots at D_80122C00. Matching slots are replaced with
 * 0xFF and the corresponding menu ID is invalidated before the final IDs are
 * written to D_80122C06.
 *
 * @note gcc272_cdk, 100% match.
 */
void func_800C6EBC(void)
{
    s32 i;
    s16 *slot;
    u8 a, b, c;
    u8 *p;
    s16 value;
    s32 invalid;

    i = 0;
    invalid = 0xFF;
    slot = D_80122C00;
    {
        s16 *idxp;
        u8 *base;

        idxp = &D_80122C10;
        base = g_menuLayoutBuffer;
        p = base + (*idxp << 6);
    }
    a = p[0xD00];
    b = p[0xD01];
    c = p[0xD02];
    do {
        value = *slot;
        if (value != invalid) {
            if (value == a) {
                *slot = invalid;
                a = 0xFF;
            } else if (value == b) {
                *slot = invalid;
                b = 0xFF;
            } else if (value == c) {
                *slot = invalid;
                c = 0xFF;
            }
        }
        i++;
        slot++;
    } while (i < 3);
    D_80122C06[0] = a;
    D_80122C06[1] = b;
    D_80122C06[2] = c;
}
