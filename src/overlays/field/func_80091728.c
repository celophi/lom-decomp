#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1;
    u16 unk2;
} PadState;

typedef struct
{
    u8 pad0[0x1C];
    u32 unk1C;
    u8 pad20[0x3A - 0x20];
    u8 unk3A;
} Arg2Struct;

extern u8 D_800EB23C[];
extern u8 D_800EB244[];
extern u8 *g_pad_ctx;

/**
 * @brief Build the active input mask for a field menu entry.
 * @param arg0 Controller slot index.
 * @param arg1 Input mapping index.
 * @param arg2 Field menu entry state.
 * @return The mapped active-button mask, or zero when input is unavailable.
 */
s32 func_80091728(s32 arg0, s32 arg1, Arg2Struct *arg2)
{
    s32 mask;
    s32 i;
    u8 want;
    volatile PadState *pad;
    u32 bits;
    u32 pad_base;
    u8 *rec_base;

    pad_base = 0x801ED600;
    if (arg2->unk3A < 3)
    {
        if ((arg2->unk1C & 0x1FF) == 0)
        {
            mask = 0;
            i = 0;
            want = D_800EB244[arg1];
            rec_base = g_pad_ctx + arg0 * 0x250;
            do
            {
                if (want == *(rec_base + i + 0x638))
                {
                    mask |= D_800EB23C[i];
                }
                i += 1;
            } while (i < 8);
            pad = (volatile PadState *)(pad_base + arg0 * 0xAE);
            if (pad->unk0 < 0xFE)
            {
                bits = (u32)pad->unk2 >> 8;
                return (((bits >> 1) & 0x20) | ((bits & 0x20) * 2) | ((bits >> 3) & 0x10) | ((bits & 0x10) * 8) | (bits & 0xF)) & mask;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}
