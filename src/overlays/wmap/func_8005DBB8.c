#include "common.h"

#define WMAP_NODE_RECORD_SIZE 12
#define WMAP_SET_BIT(index) D_800460AC[(index) / 32] = D_800460AC[(index) / 32] | (1 << ((index) % 32))

extern u8 D_800432C8[];
extern s32 D_80043454;
extern u32 D_800460AC[];
extern s32 D_800460EC;
extern s32 D_800460FC;

/**
 * @brief Rebuild the world-map reachability bitmaps from the per-node flag bytes.
 */
void func_8005DBB8(void)
{
    s32 i;

    for (i = 0; i < 0x21; i++)
    {
        if ((D_800432C8[i * WMAP_NODE_RECORD_SIZE] >> 1) & 1)
        {
            WMAP_SET_BIT(i + 0x200);
        }
    }
    if (D_80043454 & 4)
    {
        D_800460EC |= 0x01000000;
    }
    for (i = 0; i < 0x21; i++)
    {
        if (D_800432C8[i * WMAP_NODE_RECORD_SIZE] & 1)
        {
            WMAP_SET_BIT(i + 0x240);
        }
        if ((D_800432C8[i * WMAP_NODE_RECORD_SIZE] >> 1) & 1)
        {
            WMAP_SET_BIT(i + 0x280);
        }
    }
    if (D_80043454 & 4)
    {
        D_800460FC |= 0x01000000;
    }
}
