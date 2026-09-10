#include "common.h"

/** @brief Two ability prerequisites and the ability unlocked when they are met. */
typedef struct
{
    u8 a0;
    u8 l0;
    u8 a1;
    u8 l1;
    u8 result;
} AbilityRule;
extern AbilityRule D_800EC55C[];

/** @brief Four ability prerequisites plus packed equipment and technique fields. */
typedef struct
{
    u8 a0;
    u8 l0;
    u8 a1;
    u8 l1;
    u8 a2;
    u8 l2;
    u8 a3;
    u8 l3;
    u8 weapon;
    u8 result;
    u8 level;
} TechniqueRule;
extern TechniqueRule D_800EC5B8[];
extern u8 D_800FD818[];
extern s32 D_80115890;
extern u16 D_80122920[];
extern u16 D_80122998;
extern u8 *g_pad_ctx;
#define BYTE(p,o) (*(u8 *)((u8 *)(p)+(o)))
#define WORD(p,o) (*(u32 *)((u8 *)(p)+(o)))

/**
 * @brief Advance active party progression and record newly unlocked abilities.
 * @note Counters saturate at 100; the progression modifier selects one or four points.
 * @note Signed division and modulo preserve the original bitmap-index arithmetic.
 * @note Each rule pass caches the context separately, as in the target.
 */
void func_800A68B4(void)
{
    AbilityRule *ability_base;
    TechniqueRule *technique_base;
    TechniqueRule *active_rule;
    s32 party_offset;
    u8 *context;
    AbilityRule *ability_rule;
    TechniqueRule *technique_rule;
    s32 temp_a0_3;
    s32 temp_a2_10;
    s32 temp_a2_5;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 temp_v0_7;
    s32 temp_v0_8;
    s32 temp_v1_7;
    s32 equipped_index;
    s32 party_byte_offset;
    s32 party_index;
    s32 technique_party_index;
    s32 var_v1;
    s32 var_v1_2;
    u32 temp_v1_2;
    u32 technique_group;
    u8 *party_flags;
    u8 *technique_party_flags;
    u8 temp_a0_2;
    u8 temp_a1_2;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a2_3;
    s32 temp_a2_4;
    s32 temp_a2_6;
    s32 temp_a2_7;
    s32 temp_a2_8;
    s32 temp_a2_9;
    u8 temp_v1_3;
    u8 var_v0;
    u8 var_v0_2;
    u8 var_v0_3;
    u8 var_v0_4;
    u8 var_v0_5;
    u8 var_v0_6;
    u8 var_v0_7;
    u8 var_v0_8;
    u8 *temp_a0;
    u8 *temp_a1;
    u8 *temp_v0_9;
    u8 *temp_v1;
    u8 *temp_v1_4;
    u8 *temp_v1_5;
    u8 *temp_v1_6;
    u8 *party_context;

    party_index = 0;
    party_byte_offset = 0;
    party_flags = D_800FD818;
    do
    {
        if (*party_flags & 1)
        {
            temp_v1 = g_pad_ctx + party_byte_offset;
            if ((u32) (BYTE(temp_v1, 0x608) & 0x7F) < 2U)
            {
                temp_v1_2 = ((u32) WORD(temp_v1, 0x654) >> 0xA) & 0x3F;
                temp_a0 = g_pad_ctx + temp_v1_2;
                if (temp_v1_2 < 0xBU)
                {
                    temp_v1_3 = BYTE(temp_a0, 0xC4);
                    equipped_index = 0;
                    if (temp_v1_3 < 0x64U)
                    {
                        if (D_80115890 != 0)
                        {
                            BYTE(temp_a0, 0xC4) = (u8) (temp_v1_3 + 4);
                            temp_v1_4 = g_pad_ctx + (((u32) WORD(g_pad_ctx + party_byte_offset, 0x654) >> 0xA) & 0x3F);
                            if ((u8) BYTE(temp_v1_4, 0xC4) >= 0x65U)
                            {
                                BYTE(temp_v1_4, 0xC4) = 0x64U;
                            }
                        }
                        else
                        {
                            BYTE(temp_a0, 0xC4) = (u8) (temp_v1_3 + 1);
                            goto initialize_equipped_loop;
                        }
                    }
                }
                else
                {
initialize_equipped_loop:
                    equipped_index = 0;
                }
                party_offset = party_byte_offset;
                do
                {
                    temp_v1_5 = g_pad_ctx + BYTE(g_pad_ctx + party_offset + equipped_index, 0x60A);
                    temp_a0_2 = BYTE(temp_v1_5, 0x6C);
                    if (temp_a0_2 < 0x64U)
                    {
                        if (D_80115890 != 0)
                        {
                            BYTE(temp_v1_5, 0x6C) = (u8) (temp_a0_2 + 4);
                            temp_v1_6 = g_pad_ctx + BYTE(g_pad_ctx + party_offset + equipped_index, 0x60A);
                            if ((u8) BYTE(temp_v1_6, 0x6C) >= 0x65U)
                            {
                                BYTE(temp_v1_6, 0x6C) = 0x64U;
                            }
                        }
                        else
                        {
                            BYTE(temp_v1_5, 0x6C) = (u8) (temp_a0_2 + 1);
                        }
                    }
                    equipped_index += 1;
                } while (equipped_index < 2);
            }
        }
        party_byte_offset += 0x250;
        party_index += 1;
        party_flags += 0x268;
    } while (party_index < 3);
    context = g_pad_ctx;
    ability_base = D_800EC55C;
    ability_rule = ability_base;
    D_80122998 = 0;
    do
    {
        temp_a2 = ability_rule->result;
        temp_v0 = (s32) temp_a2 / 32;
        if (!(WORD((temp_v0 * 4) + context, 0x60) & (1 << (temp_a2 % 32))))
        {
            temp_a2_2 = ability_rule->a0;
            if (temp_a2_2 != 0xFF)
            {
                temp_v0_2 = (s32) temp_a2_2 / 32;
                if ((WORD((temp_v0_2 * 4) + context, 0x60) & (1 << (temp_a2_2 % 32))) &&
                ((u8) BYTE(context + temp_a2_2, 0x6C) >= (u8) ability_rule->l0))
                {
                    goto check_second_ability;
                }
            }
            else
            {
check_second_ability:
                temp_a2_3 = ability_rule->a1;
                if (temp_a2_3 != 0xFF)
                {
                    temp_v0_3 = (s32) temp_a2_3 / 32;
                    if ((WORD((temp_v0_3 * 4) + context, 0x60) & (1 << (temp_a2_3 % 32))) &&
                    ((u8) BYTE(context + temp_a2_3, 0x6C) >= (u8) ability_rule->l1))
                    {
                        goto unlock_ability;
                    }
                }
                else
                {
unlock_ability:
                    temp_a2_4 = ability_rule->result;
                    temp_v0_4 = (s32) temp_a2_4 / 32;
                    temp_a1 = (temp_v0_4 * 4) + context;
                    WORD(temp_a1, 0x60) = (s32) (WORD(temp_a1, 0x60) | (1 << (temp_a2_4 % 32)));
                    D_80122920[D_80122998] = (s16) temp_a2_4;
                    D_80122998 += 1;
                }
            }
        }
        ability_rule++;
    } while ((s32) ability_rule < (s32) (ability_base + 18));
    context = g_pad_ctx;
    technique_base = D_800EC5B8;
    technique_rule = technique_base;
    do
    {
        technique_group = (u8) technique_rule->weapon >> 4;
        temp_a2_5 = technique_rule->result & 0x7F;
        var_v1 = temp_a2_5;
        if (temp_a2_5 < 0)
        {
            var_v1 = temp_a2_5 + 0x1F;
        }
        if (!(WORD((technique_group * 4) + context, 0x34) & (1 << (temp_a2_5 % 32))))
        {
            temp_a2_6 = technique_rule->a0;
            if (temp_a2_6 != 0xFF)
            {
                temp_v0_5 = (s32) temp_a2_6 / 32;
                if ((WORD((temp_v0_5 * 4) + context, 0x60) & (1 << (temp_a2_6 % 32))) &&
                ((u8) BYTE(context + temp_a2_6, 0x6C) >= (u8) technique_rule->l0))
                {
                    goto check_second_prerequisite;
                }
            }
            else
            {
check_second_prerequisite:
                temp_a2_7 = technique_rule->a1;
                if (temp_a2_7 != 0xFF)
                {
                    temp_v0_6 = (s32) temp_a2_7 / 32;
                    if ((WORD((temp_v0_6 * 4) + context, 0x60) & (1 << (temp_a2_7 % 32))) &&
                    ((u8) BYTE(context + temp_a2_7, 0x6C) >= (u8) technique_rule->l1))
                    {
                        goto check_third_prerequisite;
                    }
                }
                else
                {
check_third_prerequisite:
                    temp_a2_8 = technique_rule->a2;
                    if (temp_a2_8 != 0xFF)
                    {
                        temp_v0_7 = (s32) temp_a2_8 / 32;
                        if ((WORD((temp_v0_7 * 4) + context, 0x60) & (1 << (temp_a2_8 % 32))) &&
                        ((u8) BYTE(context + temp_a2_8, 0x6C) >= (u8) technique_rule->l2))
                        {
                            goto check_fourth_prerequisite;
                        }
                    }
                    else
                    {
check_fourth_prerequisite:
                        temp_a2_9 = technique_rule->a3;
                        if (temp_a2_9 != 0xFF)
                        {
                            temp_v0_8 = (s32) temp_a2_9 / 32;
                            if ((WORD((temp_v0_8 * 4) + context, 0x60) & (1 << (temp_a2_9 % 32))) &&
                            ((u8) BYTE(context + temp_a2_9, 0x6C) >= (u8) technique_rule->l3))
                            {
                                goto check_party_equipment;
                            }
                        }
                        else
                        {
check_party_equipment:
                            active_rule = technique_rule;
                            technique_party_index = 0;
                            party_context = context;
                            technique_party_flags = D_800FD818;
                            temp_a2_10 = technique_rule->result & 0x7F;
                            do
                            {
                                if ((*technique_party_flags & 1) &&
                                ((u32) (BYTE(party_context, 0x608) & 0x7F) < 2U))
                                {
                                    temp_a1_2 = active_rule->weapon;
                                    temp_v1_7 = ((u32) WORD(party_context, 0x654) >> 0xA) & 0x3F;
                                    if (temp_v1_7 == (temp_a1_2 & 0xF))
                                    {
                                        var_v1_2 = temp_a2_10;
                                        if ((u8) BYTE(context + temp_v1_7, 0xC4) >= (u8) active_rule->level)
                                        {
                                            if (temp_a2_10 < 0)
                                            {
                                                var_v1_2 = temp_a2_10 + 0x1F;
                                            }
                                            temp_a0_3 = 1 << (temp_a2_10 % 32);
                                            if (!(WORD((technique_group * 4) + context, 0x34) & temp_a0_3))
                                            {
                                                technique_group = temp_a1_2 >> 4;
                                                temp_v0_9 = (technique_group * 4) + context;
                                                WORD(temp_v0_9, 0x34) = (s32) (WORD(temp_v0_9, 0x34) | temp_a0_3);
                                                if (!(active_rule->result & 0x80))
                                                {
                                                    D_80122920[D_80122998] = ((technique_group * 0x18) + temp_a2_10) | 0x8000;
                                                    D_80122998 += 1;
                                                }
                                            }
                                        }
                                    }
                                }
                                party_context += 0x250;
                                technique_party_index += 1;
                                technique_party_flags += 0x268;
                            } while (technique_party_index < 3);
                        }
                    }
                }
            }
        }
        technique_rule++;
    } while ((s32) technique_rule < (s32) (technique_base + 227));
}
