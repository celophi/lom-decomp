#include "common.h"

/** @brief Partial StateB80087FC0 layout used by func_80087FC0. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} StateB80087FC0;

/** @brief Partial RecordB80087FC0 layout used by func_80087FC0. */
typedef struct
{
    u8 pad0[0x10];
    u16 unk10;
    u8 pad12[0x1C - 0x12];
    u32 unk1C;
    u8 pad20[0x28 - 0x20];
    u8 unk28;
    u8 pad29[0x3A - 0x29];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} RecordB80087FC0;

/** @brief Partial PadCtxB80087FC0 layout used by func_80087FC0. */
typedef struct
{
    u8 pad0[0x28];
    u32 unk28;
} PadCtxB80087FC0;

/** @brief Partial FixedB80087FC0 layout used by func_80087FC0. */
typedef struct
{
    u8 pad0[0x13E];
    u8 unk13E;
} FixedB80087FC0;

extern StateB80087FC0 D_80105AE0[];
extern RecordB80087FC0 D_800FDF58[];
extern PadCtxB80087FC0 *g_pad_ctx;

void func_8008C7A8(s32 arg0, void *arg1, void *arg2);

/**
 * @brief Find an actor by key and update its nine-bit control mode.
 * @param arg0 Actor-slot lookup key.
 * @param arg1 New mode; only its low nine bits are used.
 * @return -1 when no actor matches, or 0 after updating the record.
 * @note WIP: additional instructions and temporary-register differences remain.
 */
s32 func_80087FC0(s32 arg0, s32 arg1)
{
    StateB80087FC0 *ra;
    RecordB80087FC0 *rb;
    RecordB80087FC0 *found;
    s32 i;
    s32 masked;
    u8 unk3a;
    volatile FixedB80087FC0 *fixed = (volatile FixedB80087FC0 *) 0x801ED600;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecordB80087FC0 *) -1;
check:
    if (found != (RecordB80087FC0 *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    found->unk1C = (found->unk1C & ~0x1FF) | (arg1 & 0x1FF);
    masked = (u16) found->unk1C & 0x1FF;
    if (masked != 1)
    {
        if (masked < 2)
        {
            if (masked == 0)
            {
                unk3a = found->unk3A;
                found->unk28 = 0xFF;
                found->unk10 = 0;
                if (unk3a == 1)
                {
                    if (!(g_pad_ctx->unk28 & 1))
                    {
                        goto zero_flag;
                    }
                    fixed->unk13E = unk3a;
                }
            }
        }
    }
    else
    {
        found->unk28 = 0xFF;
        found->unk10 = 0;
        func_8008C7A8(1, ra, rb);
        if (found->unk3A == masked)
        {
        zero_flag:
            fixed->unk13E = 0;
        }
    }
    return 0;
}
