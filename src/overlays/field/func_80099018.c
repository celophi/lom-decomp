#include "common.h"
/** @brief Position, state and presence fields in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y, z;
    u8 padc[0x10];
    u32 flags;
    u8 unk20, state;
    u8 pad22[3];
    u8 presence, effect;
    u8 pad27[3];
    s16 kind;
    u8 pad2c[0xE];
    u8 index;
    u8 pad3b[0x19];
} FieldHitActor;
/** @brief Animation flags controlling which actor group is eligible. */
typedef struct
{
    u8 pad0[0x18];
    u16 flags;
} FieldHitResource;
/** @brief Animation owner, recorded contacts and effect selector. */
typedef struct
{
    u8 pad0[0xC];
    FieldHitResource *resource;
    u8 pad10[0x16];
    u8 effect;
    u8 pad27[0x201];
    u8 owner_index;
    u8 contacts[9];
    u8 contact_count;
    u8 pad233[7];
    u8 contact_flags;
    u8 pad23b;
} FieldHitAnimation;
void func_8008A840(s32, s32);
void func_8008A9D8(s32, s32, s32);
void func_8008BC5C(FieldHitActor *);
void func_800A2DD8(s32);
extern FieldHitActor D_800FDF58[];
extern u8 D_80105880[], D_80105AE0[];
extern s32 D_800FE754, D_8010D020;
/**
 * @brief Collect eligible actors intersecting an animation's expanded hit bounds.
 * @param actor Actor supplying the attack origin and orientation.
 * @param radius Expansion applied to the candidate's projected bounds.
 * @param animation Animation owner, contact list and reaction selector.
 * @note state_mode points twelve bytes into each 0x23C-byte state; candidate_z
 *       points eight bytes into each 0x54-byte actor. Byte views preserve packed
 *       flag/count words and signed halfword bounds that overlap word checks.
 */
void func_80099018(FieldHitActor *actor, s32 radius, FieldHitAnimation *animation)
{
    s32 temp_a0_7;
    s32 temp_a0_8;
    s16 temp_v1;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 var_a2_2;
    s32 var_a3;
    s32 var_t0;
    s32 var_t1;
    s32 *candidate_position;
    s32 *candidate_record;
    s32 temp_a0_10;
    s32 temp_a0_4;
    s32 temp_a0_5;
    s32 temp_a0_9;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v1_2;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 var_a0;
    s32 var_a2;
    s32 end_index;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 candidate_index;
    u8 temp_a0;
    u8 temp_a0_2;
    u8 temp_a0_3;
    u8 temp_a0_6;
    u8 temp_v0_6;
    u8 temp_v1_7;
    u8 *temp_a1_2;
    u8 *temp_v0;
    u8 *temp_v0_4;
    u8 *temp_v0_5;
    u8 *state_mode;
    u8 *candidate_z;
    u8 *var_v0;
    u8 *var_v0_5;

    if (D_8010D020 != 0)
    {
        candidate_index = 0;
        end_index = 0xD;
    }
    else
    {
        if (animation->resource->flags & 1)
        {
            candidate_index = 0;
            if ((u8)animation->owner_index >= 3U)
            {
                candidate_index = 3;
                goto block_6;
            }
            goto block_8;
        }
        candidate_index = 3;
        if ((u8)animation->owner_index < 3U)
        {
        block_6:
            end_index = 0xD;
        }
        else
        {
            candidate_index = 0;
        block_8:
            end_index = 3;
        }
    }
    candidate_position = (s32 *)((u8 *)D_800FDF58 + candidate_index * 0x54);
    state_mode = (candidate_index * 0x23C) + D_80105AE0;
    if (candidate_index < end_index)
    {
        state_mode += 0xC;
        candidate_z = (u8 *)candidate_position + 8;
        candidate_record = candidate_position;
    next_candidate:
    {
        if (*(s32 *)(state_mode + (364)) & 0x80)
        {
            temp_a0 = actor->index;
            var_a2 = 0;
            if (D_800FDF58[temp_a0].kind == 0x91)
            {
                temp_v0 = (temp_a0 * 0x23C) + D_80105AE0;
                temp_a0_2 = *(u8 *)(temp_v0 + (379));
                var_v1 = 0;
                if (temp_a0_2 != 0)
                {
                loop_15:
                    var_v0 = temp_v0 + var_v1;
                    var_v1 += 1;
                    if (*(u8 *)(var_v0 + (384)) != candidate_index)
                    {
                        if (var_v1 >= (s32)temp_a0_2)
                        {
                        }
                        else
                        {
                            goto loop_15;
                        }
                    }
                    else
                    {
                        goto block_18;
                    }
                }
            }
        }
        else
        {
        block_18:
            var_a2 = 1;
        }
        temp_a0_3 = animation->owner_index;
        if ((candidate_index != temp_a0_3) &&
            ((*(s32 *)(state_mode + (316)) != 0) || (*(s32 *)(state_mode + (324)) != 0)) &&
            ((*(s32 *)(state_mode + (308)) != 0) || (*(s32 *)(state_mode + (312)) != 0)) &&
            (*(s32 *)(state_mode + (288)) != 0) && (temp_v1 = *(s16 *)(candidate_z + (34)), (temp_v1 != 0x91)) &&
            (temp_v1 != 0xAE) && (temp_v1 != 0x87) && (*(u8 *)(candidate_z + (29)) != 0xFF) &&
            (temp_a0_3 != candidate_index) && (*(s32 *)(state_mode + (-8)) != 0) &&
            (temp_a0_4 = *(s32 *)(state_mode + (364)), ((temp_a0_4 & 1) == 0)) &&
            ((var_v0_2 = temp_a0_4 & 0x20, ((candidate_index < 3) != 0)) ||
             (var_v0_2 = temp_a0_4 & 0x20, ((*(s32 *)(state_mode + (4)) & 0xF) == D_800FE754))) &&
            (var_v0_2 == 0) && (var_a2 != 0) && !(*(s32 *)(state_mode + (360)) & 0x8000))
        {
            if (!(temp_a0_4 & 0x40))
            {
                if ((u8) * (u8 *)(candidate_z + (50)) < 2U)
                {
                    var_v0_3 = *(u8 *)(candidate_z + (50)) * 0x1C;
                }
                else
                {
                    var_v0_3 = 0x38;
                }
                temp_a0_5 = *(s32 *)(D_80105880 + var_v0_3 + 0xC);
                if (temp_a0_5 == *(u8 *)(candidate_z + (50)))
                {
                    if ((u32)(temp_a0_5 & 0xFF) < 2U)
                    {
                        var_v0_4 = temp_a0_5 * 0x1C;
                    }
                    else
                    {
                        var_v0_4 = 0x38;
                    }
                    if (*(s32 *)(D_80105880 + var_v0_4) == 0)
                    {
                        goto block_46;
                    }
                    goto block_89;
                }
                goto block_46;
            }
        block_46:
            if (!(*(s32 *)(state_mode + (0)) & 0x2280))
            {
                temp_a0_6 = animation->contact_count;
                var_v1_2 = 0;
                if (temp_a0_6 != 0)
                {
                loop_49:
                    var_v0_5 = (u8 *)animation + var_v1_2;
                    if (candidate_index != *(u8 *)(var_v0_5 + (553)))
                    {
                        var_v1_2 += 1;
                        if (var_v1_2 < (s32)temp_a0_6)
                        {
                            goto loop_49;
                        }
                    }
                }
                if (var_v1_2 == animation->contact_count)
                {
                    temp_v1_2 = *(s32 *)(candidate_z + (0));
                    temp_v0_2 = actor->z;
                    temp_a1 = temp_v1_2 - temp_v0_2;
                    var_v1_3 = (temp_v1_2 - temp_v0_2) / 384;
                    if (var_v1_3 < 0)
                    {
                        var_v1_3 = -var_v1_3;
                    }
                    if (var_v1_3 < (radius + ((s32)(*(u16 *)(state_mode + (290)) << 0x10) >> 0x11)))
                    {
                        temp_a0_7 = *(s16 *)(state_mode + 0x134);
                        temp_v1_3 = *(s16 *)(state_mode + 0x138);
                        if (temp_a0_7 < temp_v1_3)
                        {
                            var_t0 = temp_a0_7;
                            var_t1 = temp_v1_3;
                        }
                        else
                        {
                            var_t0 = temp_v1_3;
                            var_t1 = temp_a0_7;
                        }
                        temp_a0_8 = *(s16 *)(state_mode + (310));
                        temp_v1_4 = *(s16 *)(state_mode + (314));
                        if (temp_a0_8 < temp_v1_4)
                        {
                            var_a2_2 = temp_a0_8;
                            var_a3 = temp_v1_4;
                        }
                        else
                        {
                            var_a2_2 = temp_v1_4;
                            var_a3 = temp_a0_8;
                        }
                        var_t0 -= radius;
                        var_t1 += radius;
                        var_a2_2 -= radius;
                        var_a3 += radius;
                        var_v0_6 = temp_a1;
                        if (temp_a1 < 0)
                        {
                            var_v0_6 = temp_a1 + 0x1FF;
                        }
                        temp_v0_3 = (s32)((var_v0_6 >> 9) + ((u32)var_v0_6 >> 0x1F)) >> 1;
                        temp_a2 = var_a2_2 - temp_v0_3;
                        temp_a3 = var_a3 - temp_v0_3;
                        temp_v1_5 = *candidate_position;
                        temp_a0_9 = actor->x;
                        if (((temp_v1_5 + (var_t0 << 8)) < temp_a0_9) && (temp_a0_9 < (temp_v1_5 + (var_t1 << 8))) &&
                            (temp_v1_6 = *(s32 *)(candidate_z + (-4)), temp_a0_10 = actor->y,
                             (((temp_v1_6 + (temp_a2 << 8)) < temp_a0_10) != 0)) &&
                            (temp_a0_10 < (temp_v1_6 + (temp_a3 << 8))) &&
                            ((u8) * (u8 *)(D_80105AE0 + animation->owner_index * 0x23C + 0x17B) < 9U))
                        {
                            *(s32 *)(state_mode + (364)) = (s32)(*(s32 *)(state_mode + (364)) | 0x80);
                            *(s32 *)(state_mode + (0)) = (s32)(*(s32 *)(state_mode + (0)) & ~0x400);
                            temp_v0_4 = (animation->owner_index * 0x23C) + D_80105AE0;
                            *(u8 *)(temp_v0_4 + temp_v0_4[0x17B] + 0x180) = candidate_index;
                            temp_v0_5 = (animation->owner_index * 0x23C) + D_80105AE0;
                            *(u8 *)(temp_v0_5 + (379)) = (u8)(*(u8 *)(temp_v0_5 + (379)) + 1);
                            animation->contact_flags = (u8)(animation->contact_flags | (1 << animation->contact_count));
                            animation->contacts[animation->contact_count] = candidate_index;
                            if (*(u8 *)(candidate_z + (25)) & 0x80)
                            {
                                var_v1_4 = *candidate_position;
                                var_a0 = actor->x;
                                var_v0_7 = animation->contact_count * 4;
                            }
                            else
                            {
                                var_v1_4 = actor->x;
                                var_a0 = *candidate_position;
                                var_v0_7 = animation->contact_count * 4;
                            }
                            *(s16 *)((u8 *)animation + var_v0_7 + 0x1FE) = (s16)((s32)(var_v1_4 - var_a0) >> 8);
                            temp_a1_2 = (u8 *)animation + (animation->contact_count * 4);
                            *(s16 *)(temp_a1_2 + (512)) = (s16)(((s32)(actor->y - *(s32 *)(candidate_z + (-4))) >> 8) -
                                                                ((s32)(actor->z - *(s32 *)(candidate_z + (0))) >> 9));
                            if (actor->flags & 0x08000000)
                            {
                                temp_v1_7 = actor->presence;
                                actor->presence = 0xFFU;
                                actor->effect = temp_v1_7;
                            }
                            animation->contact_count = (u8)(animation->contact_count + 1);
                            func_8008BC5C((FieldHitActor *)candidate_record);
                            if ((candidate_index < 2) && !(((FieldHitActor *)candidate_record)->flags & 0x1FF))
                            {
                                func_800A2DD8(candidate_index);
                            }
                            if (((u8)animation->effect < 0xCU) || (D_800FDF58[animation->owner_index].kind == 0xBC))
                            {
                                func_8008A9D8(animation->owner_index, candidate_index, animation->effect);
                                candidate_record += 21;
                            }
                            else
                            {
                                temp_v0_6 = animation->effect;
                                switch (temp_v0_6)
                                {
                                case 0x34:
                                    func_8008A9D8(animation->owner_index, candidate_index, 0x16U);
                                    candidate_record += 21;
                                    break;
                                case 0x50:
                                    func_8008A9D8(animation->owner_index, candidate_index, 0x12U);
                                    candidate_record += 21;
                                    break;
                                case 0x51:
                                    func_8008A9D8(animation->owner_index, candidate_index, 0x13U);
                                    candidate_record += 21;
                                    break;
                                case 0x4E:
                                    func_8008A9D8(animation->owner_index, candidate_index, 0x14U);
                                    candidate_record += 21;
                                    break;
                                case 0x4F:
                                    func_8008A9D8(animation->owner_index, candidate_index, 0x15U);
                                    candidate_record += 21;
                                    break;
                                case 0x3E:
                                    func_8008A9D8(animation->owner_index, candidate_index, 0x19U);
                                    candidate_record += 21;
                                    break;
                                case 0x45:
                                    func_8008A9D8(animation->owner_index, candidate_index, 0x1AU);
                                    candidate_record += 21;
                                    break;
                                default:
                                    func_8008A840(animation->owner_index, candidate_index);
                                    goto block_89;
                                }
                            }
                        }
                        else
                        {
                            goto block_89;
                        }
                    }
                    else
                    {
                        goto block_89;
                    }
                }
                else
                {
                    goto block_89;
                }
            }
            else
            {
                goto block_89;
            }
        }
        else
        {
        block_89:
            candidate_record += 21;
        }
        candidate_index += 1;
        candidate_z += 0x54;
        candidate_position += 21;
        state_mode += 0x23C;
    }
        if (candidate_index < end_index)
        {
            goto next_candidate;
        }
    }
}
