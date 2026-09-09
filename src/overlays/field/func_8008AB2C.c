#include "common.h"

/** @brief Field entry with animation state and actor index. */
typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[14];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Entry;
/** @brief Actor record with lookup ID, state flags, and action byte. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC, unk10, unk14;
    u8 pad18[0x16F - 0x18];
    u8 unk16F;
    u8 pad170[8];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Actor;
/** @brief Actor data record containing the pending action byte. */
typedef struct
{
    u8 pad0[0x259];
    u8 unk259;
    u8 pad25A[14];
} Data;
/** @brief Input context view containing the actor-zero event counter. */
typedef struct
{
    u8 pad0[0x3154];
    s32 unk3154;
} Pad;
extern Entry D_800FDF58[];
extern Actor D_80105AE0[];
extern Data D_800FD818[];
extern Pad *g_pad_ctx;
extern s32 D_8010A000;
extern void func_8008B870(Entry *);
/**
 * @brief Mark an actor for an action and dispatch its state-dependent update.
 * @param arg0 Actor ID to locate among the field records.
 * @return Zero when found, or -1 when no record has the requested ID.
 */
s32 func_8008AB2C(s32 arg0)
{
    Entry *var_a0;
    Actor *var_a2;
    Entry *var_a2_2;
    s16 temp_v1;
    s32 temp_v0;
    s32 var_v1;
    s32 temp_v1_2;
    Actor *temp_v0_2;
    u8 *actor_base;

    var_a0 = D_800FDF58;
    var_a2 = D_80105AE0;
    var_v1 = 0;
loop_1:
    var_v1 += 1;
    if (var_a2->unk14 != arg0)
    {
        var_a2++;
        var_a0++;
        if (var_v1 >= 0xD)
        {
            var_a2_2 = (Entry *)-1;
        }
        else
        {
            goto loop_1;
        }
    }
    else
    {
        goto found;
    }
check:
    if (var_a2_2 != (Entry *)-1)
    {
        goto body;
    }
    return -1;
found:
    var_a2_2 = var_a0;
    goto check;
body:
    if (var_a2_2->unk3A == 0)
    {
        temp_v0 = g_pad_ctx->unk3154;
        if (temp_v0 != -1)
        {
            g_pad_ctx->unk3154 = (s32)(temp_v0 + 1);
        }
    }
    actor_base = (u8 *)D_80105AE0;
    temp_v0_2 = (Actor *)(actor_base + var_a2_2->unk3A * 0x23C);
    temp_v0_2->unkC = (s32)(temp_v0_2->unkC | 0x10000000);
    if ((u8)var_a2_2->unk3A < 3U)
    {
        D_800FD818[var_a2_2->unk3A].unk259 = 5;
    }
    else if (((Actor *)(actor_base + var_a2_2->unk3A * 0x23C))->unk8 < 0)
    {
        D_8010A000 = 5;
    }
    temp_v1 = var_a2_2->unk2A;
    if (temp_v1 == 0x87)
    {
        return 0;
    }
    if (temp_v1 >= 0x88)
    {
        if (temp_v1 == 0x91)
        {
            return 0;
        }
        goto finish;
    }
    if (temp_v1 >= 0x85)
    {
        actor_base = (u8 *)D_80105AE0;
        switch (((Actor *)(actor_base + var_a2_2->unk3A * 0x23C))->unk16F)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 10:
            break;
        default:
            actor_base = (u8 *)D_80105AE0;
            if (!(((u32)((Actor *)(actor_base + var_a2_2->unk3A * 0x23C))->unk178 >> 6) & 1))
            {
                return 0;
            }
            break;
        }
    }
finish:
    func_8008B870(var_a2_2);
    return 0;
}
