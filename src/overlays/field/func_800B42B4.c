#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 *unk4;
    u8 *unk8;
    u8 unkC[8];
    s32 unk14;
    s32 unk18;
} StructB3580;

extern StructB3580 *D_80123FB0;

s32 func_800B3670(s32 arg0);

/**
 * @brief Derive a track-tempo-scaled value from an 8-bit add/sub bitmask pair
 * against D_80123FB0's per-index byte table, clamped to a 4..10 range and
 * scaled by func_800B3670's result.
 * @param arg0 Record with add/sub bitmasks at 0x15/0x16 and a mode byte at 0x3F.
 * @return Value clamped to the range 1..99.
 */
s32 func_800B42B4(u8 *arg0)
{
    u32 add_mask;
    u32 sub_mask;
    u32 count;
    s32 value;
    s32 clamped;
    u32 scaled;

    count = 0;
    value = 8;
    add_mask = arg0[0x15];
    sub_mask = arg0[0x16];

    for (; count < 8; count++)
    {
        if (add_mask & 1)
        {
            value += D_80123FB0->unkC[count];
        }
        if (sub_mask & 1)
        {
            value -= D_80123FB0->unkC[count];
        }
        add_mask >>= 1;
        sub_mask >>= 1;
    }

    if (value < 5)
    {
        clamped = 4;
    }
    else
    {
        clamped = value;
        if (clamped >= 0xB)
        {
            clamped = 0xA;
        }
    }
    value = clamped;

    scaled = (u32)(func_800B3670(arg0[0x3F] & 4) * value) >> 3;

    if (scaled >= 2)
    {
        if (scaled >= 0x64)
        {
            scaled = 0x63;
        }
    }
    else
    {
        scaled = 1;
    }

    return scaled;
}
