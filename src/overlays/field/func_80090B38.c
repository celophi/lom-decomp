#include "common.h"

/** @brief Animation entry containing eligibility state and actor slot. */
typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[4];
    s16 unk2A;
    u8 pad2C[14];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Entry;
/** @brief Actor record containing resources, group, and eligibility flags. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    u8 pad14[0x12C - 0x14];
    s32 unk12C;
    u8 pad130[0x174 - 0x130];
    s32 unk174;
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Actor;
extern Entry D_800FDF58[];
extern Actor D_80105AE0[];
extern s32 D_800FE754;
extern u8 D_80105880[];
/**
 * @brief Compact eligible actor indices into the front of the supplied list.
 * @param arg0 Number of indices to inspect.
 * @param arg1 Input indices and destination for the eligible indices.
 * @return Number of eligible indices copied.
 * @note The original routine reserves scratch space for 16 eligible entries.
 */
s32 func_80090B38(s32 arg0, s32 *arg1)
{
    Entry *entry_base;
    Actor *actor_base;
    u8 *slot_base;
    s32 group;
    s32 sentinel;
    s32 scratch[16];
    s16 temp_v1_2;
    s32 *var_a1;
    s32 *var_t2;
    s32 *var_t4;
    s32 *var_v1;
    s32 temp_a2_2;
    s32 slot;
    s32 temp_t0;
    s32 temp_v0;
    s32 temp_v1;
    s32 temp_v1_3;
    s32 var_t1;
    s32 var_t1_2;
    s32 var_t3;
    s32 var_v0;
    s32 var_v0_2;
    Entry *temp_a2;
    Actor *temp_a3;

    var_a1 = arg1;
    var_t1 = 0;
    var_t3 = var_t1;
    if (arg0 > 0)
    {
        sentinel = 0xFF;
        entry_base = D_800FDF58;
        actor_base = D_80105AE0;
        group = D_800FE754;
        slot_base = D_80105880;
        var_t2 = var_a1;
        var_t4 = scratch;
        do
        {
            temp_v1 = *var_t2;
            if (temp_v1 != sentinel)
            {
                temp_a2 = (Entry *)(temp_v1 * 0x54 + (s32)entry_base);
                temp_a3 = (Actor *)(temp_v1 * 0x23C + (s32)actor_base);
                if ((temp_a2->unk25 != sentinel) && (temp_a3->unk4 != 0))
                {
                    temp_t0 = temp_a3->unk178;
                    if (!(temp_t0 & 1) && ((var_t1 < 3) || ((temp_a3->unk10 & 0xF) == group)))
                    {
                        temp_v1_2 = temp_a2->unk2A;
                        if ((temp_v1_2 != 0x91) && (temp_v1_2 != 0xAE) && (temp_v1_2 != 0x87))
                        {
                            if (!(temp_t0 & 0x40))
                            {
                                if ((u8)temp_a2->unk3A < 2U)
                                {
                                    var_v0 = temp_a2->unk3A * 0x1C;
                                }
                                else
                                {
                                    var_v0 = 0x38;
                                }
                                slot = temp_a2->unk3A;
                                temp_a2 = (Entry *)*(s32 *)(slot_base + var_v0 + 0xC);
                                temp_a2_2 = (s32)temp_a2;
                                if (temp_a2_2 == slot)
                                {
                                    if ((u32)(temp_a2_2 & 0xFF) < 2U)
                                    {
                                        var_v0_2 = temp_a2_2 * 0x1C;
                                    }
                                    else
                                    {
                                        var_v0_2 = 0x38;
                                    }
                                    if (*(s32 *)(slot_base + var_v0_2) == 0)
                                    {
                                        goto block_20;
                                    }
                                }
                                else
                                {
                                    goto block_20;
                                }
                            }
                            else
                            {
                            block_20:
                                if ((temp_a3->unk12C != 0) && !(temp_a3->unkC & 0x2280))
                                {
                                    temp_v1_3 = temp_a3->unk178;
                                    if (!(temp_v1_3 & 0x20) && !((u8)temp_v1_3 & 0x80) &&
                                        !(temp_a3->unk174 & 0x8000))
                                    {
                                        var_t3 += 1;
                                        *var_t4 = *var_t2;
                                        var_t4++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            var_t1 += 1;
            var_t2++;
        } while (var_t1 < arg0);
    }
    var_t1 = 0;
    if (var_t3 > 0)
    {
        var_v1 = scratch;
        do
        {
            temp_v0 = *var_v1;
            var_v1++;
            var_t1 += 1;
            *var_a1 = temp_v0;
            var_a1++;
        } while (var_t1 < var_t3);
    }
    return var_t3;
}
