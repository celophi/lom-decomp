#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    s16 unk10;
    s16 unk12;
    s32 unk14;
    s16 unk18;
    s16 unk1A;
    s32 unk1C;
    s16 unk20;
    s16 unk22;
} CardaPolyG4Words;

extern s32 D_80165FEC;
extern s32 D_80166078;
extern s32 D_80166ADC;
extern s32 D_80166B8C;

/**
 * @brief Build and link CARDA's memory-card progress-bar gouraud quad.
 *
 * The progress rate is mode-dependent, then clamped to 0x100 before being
 * converted to the quad's right-edge extent.
 * @see matching: 100.00%
 */
s32 func_8014385C(s32 arg0, s32 *arg1)
{
    CardaPolyG4Words *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    if (D_80166ADC != 0)
    {
        elapsed = func_8002054C(-1) - D_80166B8C;
        if ((u32)(D_80166078 - 2) < 2)
        {
            if (D_80165FEC == 0xF4)
            {
                elapsed *= 0x10;
            }
            else
            {
                elapsed /= 3;
            }
        }
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        ((CardaPolyG4Words *)arg0)->unk4 = 0xFF;
        ((CardaPolyG4Words *)arg0)->unkC = 0xFFFF;
        ((CardaPolyG4Words *)arg0)->unk1C = 0xFF0000;
        ((u8 *)arg0)[3] = 8;
        ((u8 *)arg0)[7] = 0x38;
        ((CardaPolyG4Words *)arg0)->unk14 = color;
        /*
         * These single-iteration scopes preserve GCC 2.7.2's reference
         * weighting for the packet pointer and the x2 store.
         */
        do
        {
            do
            {
                g = (CardaPolyG4Words *)arg0;
                ((CardaPolyG4Words *)arg0)->unk18 = 0;
            } while (0);
            extent = elapsed * 0x120;
            ((CardaPolyG4Words *)arg0)->unk8 = 0;
            if (extent < 0)
            {
                g = (CardaPolyG4Words *)arg0;
                extent += 0xFF;
            }
            ((CardaPolyG4Words *)arg0)->unk20 = extent >> 8;
        } while (0);
        ((CardaPolyG4Words *)arg0)->unk10 = extent >> 8;
        arg0 = (s32)((u8 *)g + 0x24);
        g->unk12 = 0;
        g->unkA = 0;
        g->unk22 = 0x48;
        g->unk1A = 0x48;
        g->unk0 = (g->unk0 & 0xFF000000) | (*arg1 & 0xFFFFFF);
        *arg1 = (*arg1 & 0xFF000000) | ((s32)g & 0xFFFFFF);
    }
    return arg0;
}
