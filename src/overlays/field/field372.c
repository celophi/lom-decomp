#include "common.h"

/** @brief D_800FDF58 entry (0x54 stride); only the actor id at 0x3A is used. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;   /* 0x3A */
    u8 pad1[0x54 - 0x3B];
} EntryA0;

/** @brief D_80105AE0 actor slot (0x23C stride); matched on the 0x14 handle. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;  /* 0x14 */
    u8 pad1[0x23C - 0x18];
} EntryA2;

/** @brief Pad context; only the retry counter at 0x3158 is touched. */
typedef struct
{
    u8 pad[0x3158];
    s32 unk3158;
} PadCtx;

extern EntryA0 D_800FDF58;
extern EntryA2 D_80105AE0;
extern PadCtx *g_pad_ctx;

extern void func_8008C024(EntryA0 *arg0, s32 arg1);

/**
 * @brief Find the D_800FDF58 entry for an actor handle and dispatch it.
 *
 * Scans up to 0xD actor slots for the one whose @c unk14 handle equals @p arg0;
 * returns -1 if none match. For a matched entry with actor id >= 3 it bumps the
 * pad retry counter, then forwards the entry and @p arg1 to func_8008C024.
 *
 * @param arg0 Actor handle to search for.
 * @param arg1 Passed through to func_8008C024.
 * @return -1 when no slot matches, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_8008AE14(s32 arg0, s32 arg1)
{
    EntryA0 *var_a0;
    EntryA2 *var_a2;
    s32 temp_v0;
    s32 var_v1;

    var_a0 = &D_800FDF58;
    var_a2 = &D_80105AE0;
    var_v1 = 0;
loop_1:
    if (var_a2->unk14 != arg0)
    {
        var_a2 += 1;
        var_v1 += 1;
        var_a0 += 1;
        if (var_v1 >= 0xD)
        {
            var_a0 = (EntryA0 *)-1;
        }
        else
        {
            goto loop_1;
        }
    }
    if (var_a0 == (EntryA0 *)-1)
    {
        return -1;
    }
    if ((u8)var_a0->unk3A >= 3U)
    {
        temp_v0 = g_pad_ctx->unk3158;
        if (temp_v0 != -1)
        {
            g_pad_ctx->unk3158 = temp_v0 + 1;
        }
    }
    func_8008C024(var_a0, arg1);
    return 0;
}
