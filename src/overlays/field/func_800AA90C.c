#include "common.h"

/** @brief Party entry with overlapping flag and kind bytes, at stride 0x268. */
typedef struct
{
    union
    {
        u16 word;
        struct
        {
            u8 flags, kind;
        } bytes;
    } head;
    u8 unk2, unk3;
    u8 pad4[0x254];
    u8 unk258;
    u8 pad259[0xF];
} Party;
/** @brief Actor flags within the 0x54-byte field record. */
typedef struct
{
    u8 pad0[0x1C];
    s32 unk1C;
    u8 pad20[0x34];
} Actor;
/** @brief Runtime health fields within the 0x23C-byte actor state. */
typedef struct
{
    s32 unk0, unk4, unk8;
    u8 padC[0x230];
} State;
/** @brief Eight-byte action descriptor with byte and halfword flag access. */
typedef struct
{
    s16 unk0;
    union
    {
        u16 word;
        struct
        {
            u8 low, high;
        } byte;
    } bits;
    s16 unk4, unk6;
} Record;
/** @brief Selected item attributes at offsets 0x24 and 0x25. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24, unk25;
} Item;
/** @brief Accessed global settings and player fields within the saved context. */
typedef struct
{
    u8 pad0[0x28];
    u32 unk28;
    u8 pad2C[0x5F0 - 0x2C];
    u8 unk5F0;
    u8 pad5F1[0x17];
    u8 unk608, unk609, unk60A, unk60B, unk60C;
    u8 pad60D[7];
    u16 unk614;
    u8 pad616[0x2A];
    u8 unk640;
    u8 pad641[0x13];
    u32 unk654;
    u8 pad658[0x1E8];
    u8 unk840;
    u8 pad841[0x17];
    u32 unk858;
} Context;
void akao_set_paused(s32);             /* extern */
void cdrom_set_audio_volume(u8, s32); /* extern */
void func_8008C7A8(void);              /* extern */
void func_80091438(s32);               /* extern */
void func_800A3D44(s32, u8);           /* extern */
void func_800A5174(s32, s32);          /* extern */
void func_800A54D0(void);              /* extern */
extern u8 D_800EB114[];
extern u8 D_800EB24C[];
extern Party D_800FD818[];
extern Actor D_800FDF58[];
extern State D_80105AE0[];
extern Record D_8010A038[];
extern s32 D_801158A0;
extern u8 *g_pad_ctx;

/**
 * @brief Apply saved player settings and rebuild the party action descriptors.
 * @param refresh_only Nonzero preserves the current party membership and health values.
 * @note Zero also reloads active-party data and palettes before refreshing actions.
 * @note Packed descriptor byte writes preserve the other flag byte.
 */
void func_800AA90C(s32 refresh_only)
{
    Party *initial_party;
    Party *party;
    s16 texture_value;
    u8 *controller;
    u8 *item_context;
    s32 context_offset;
    s32 record_offset;
    s32 absent;
    u8 selected;
    s32 first_one;
    s32 first_two;
    Record *record_base;
    u8 **context_pointer;
    u8 *pair_first;
    u8 *pair_second;
    Context *input_base;
    s32 temp_a1;
    s32 controller_or_player_test;
    s32 var_a0_2;
    s32 var_a1;
    s32 button_index;
    s32 var_a2;
    s32 var_a3;
    s32 actor_stride_words;
    s32 player_index;
    s32 context_stride;
    s32 record_stride;
    s32 state_stride;
    s32 item_record_offset;
    u16 temp_a0_2;
    u16 temp_a2;
    u32 temp_v1_4;
    u8 temp_a1_3;
    u8 temp_v0;
    u8 temp_v1_6;
    Actor *temp_a0;
    Record *temp_a0_3;
    Item *temp_a0_4;
    Record *temp_a0_5;
    Context *temp_a1_2;
    Record *temp_a3;
    Context *temp_t4;
    Record *temp_v0_2;
    Context *temp_v1;
    State *temp_v1_2;
    Context *temp_v1_3;
    Context *temp_v1_5;
    Context *item_cursor;

    akao_set_paused((((u32)((Context *)g_pad_ctx)->unk28 >> 1) & 1) ^ 1);
    cdrom_set_audio_volume(0x7F, ((u32)((Context *)g_pad_ctx)->unk28 >> 1) & 1);
    controller_or_player_test = (s32)0x801ED600;
    ((u8 *)controller_or_player_test)[0x90] = (s8)(*(volatile u32 *)&((Context *)g_pad_ctx)->unk28 & 1);
    if ((((Context *)g_pad_ctx)->unk858 & 0x80) && (((Context *)g_pad_ctx)->unk840 != 0))
    {
        ((u8 *)controller_or_player_test)[0x13E] = (s8)(*(volatile u32 *)&((Context *)g_pad_ctx)->unk28 & 1);
    }
    else
    {
        ((u8 *)controller_or_player_test)[0x13E] = 0;
    }
    player_index = 0;
    if (refresh_only == 0)
    {
        first_two = 2;
        first_one = 1;
        initial_party = D_800FD818;
        var_a3 = player_index;
    first_party:
    {
        if (((Context *)(g_pad_ctx + var_a3))->unk5F0 != 0)
        {
            initial_party->head.bytes.kind = 0xFF;
            temp_a2 = initial_party->head.word | 1;
            temp_v1 = (Context *)(g_pad_ctx + var_a3);
            initial_party->head.word = temp_a2;
            temp_a1 = temp_v1->unk608 & 0x7F;
            if (temp_a1 < 2)
            {
                initial_party->head.word = (u16)((temp_a2 & 0xFFFD) | ((temp_a1 & 1) * 2));
                initial_party->unk3 = 0;
            }
            else if (temp_a1 == first_two)
            {
                initial_party->head.word = (u16)(temp_a2 & 0xFFFD);
                selected = temp_v1->unk609;
                initial_party->unk3 = first_one;
                initial_party->unk2 = selected;
            }
            else if (temp_a1 == 3)
            {
                initial_party->head.word = (u16)(temp_a2 & 0xFFFD);
                selected = temp_v1->unk609;
                initial_party->unk3 = first_two;
                initial_party->unk2 = selected;
            }
            else if (temp_a1 == 4)
            {
                initial_party->head.word = (u16)(temp_a2 & 0xFFFD);
                selected = temp_v1->unk609;
                initial_party->unk3 = first_two;
                initial_party->unk2 = selected + 0x41;
            }
        }
        else
        {
            initial_party->unk3 = first_one;
            initial_party->unk2 = 0U;
            initial_party->head.word = (u16)(initial_party->head.word & 0xFFFE);
        }
        initial_party++;
        player_index += 1;
        var_a3 += 0x250;
    }
        if (player_index < 3)
        {
            goto first_party;
        }
        player_index = 0;
        func_800A54D0();
    }
    context_pointer = &g_pad_ctx;
    record_base = D_8010A038;
    party = D_800FD818;
    actor_stride_words = player_index;
    record_stride = player_index;
    context_stride = player_index;
    state_stride = player_index;
party_loop:
{
    if (party->head.bytes.flags & 1)
    {
        temp_a0 = (Actor *)((actor_stride_words + player_index) * 4 + (u8 *)D_800FDF58);
        temp_a1_2 = (Context *)((*context_pointer) + context_stride);
        temp_a0->unk1C = (s32)((temp_a0->unk1C & ~0x1FF) | (((u8)temp_a1_2->unk608 >> 7) ^ 1));
        temp_v0 = ((u32)temp_a1_2->unk654 >> 0xA) & 0x3F;
        controller_or_player_test = player_index < 2;
        if (party->head.bytes.kind != temp_v0)
        {
            party->head.bytes.kind = temp_v0;
            if (controller_or_player_test != 0)
            {
                func_80091438(player_index);
            }
            if (refresh_only == 0)
            {
                temp_v1_2 = (State *)(state_stride + (u8 *)D_80105AE0);
                temp_a0_2 = ((Context *)((*context_pointer) + context_stride))->unk614;
                temp_v1_2->unk8 = (s32)((temp_v1_2->unk8 & 0xFF000000) | temp_a0_2);
                temp_v1_2->unk0 = (s32)temp_a0_2;
                temp_v1_2->unk4 = (s32)temp_a0_2;
            }
            if ((D_801158A0 != 0) && (refresh_only != 0) && (controller_or_player_test != 0))
            {
                func_800A3D44(player_index, party->head.bytes.kind);
            }
        }
        party->unk258 = 0;
        if ((u32)(party->head.bytes.kind - 1) < 2U)
        {
            party->unk258 = 1;
            var_a1 = 1;
            var_a0_2 = context_stride + 0x40;
            do
            {
                temp_v1_3 = (Context *)((*context_pointer) + var_a0_2);
                if (temp_v1_3->unk640 != 0)
                {
                    temp_v1_4 = temp_v1_3->unk654;
                    if ((((temp_v1_4 >> 8) & 3) == 1) && !((temp_v1_4 >> 0xA) & 0x3F))
                    {
                        party->unk258 = 0;
                    }
                }
                var_a1 += 1;
                var_a0_2 += 0x40;
            } while (var_a1 < 4);
        }
        if (player_index == 2)
        {
            button_index = 0;
            if (D_800FD818[2].unk3 == player_index)
            {
                func_800A5174(2, D_800FD818[2].unk2 + 0xA9B);
                party++;
            }
            else
            {
                goto block_40;
            }
        }
        else
        {
            button_index = 0;
        block_40:
            pair_first = D_800EB114;
            pair_second = D_800EB114 + 1;
            input_base = (Context *)((*context_pointer) + context_stride);
            var_a2 = record_stride;
            do
            {
                temp_v1_5 = (Context *)((u8 *)input_base + button_index);
                temp_a0_3 = (Record *)(var_a2 + (u8 *)record_base);
                temp_a0_3->unk0 = (s16)temp_v1_5->unk60A;
                temp_a0_3->unk4 = (s16) * ((temp_v1_5->unk60A * 2) + pair_first);
                button_index += 1;
                temp_a0_3->unk6 = (s16) * ((temp_v1_5->unk60A * 2) + pair_second);
                var_a2 += 8;
            } while (button_index < 2);
            context_offset = context_stride;
            absent = 0xFF;
            record_offset = record_stride;
            item_context = *context_pointer;
            item_record_offset = 0x20;
            temp_t4 = (Context *)(item_context + context_offset);
            item_cursor = temp_t4;
        item_loop:
        {
            if (item_cursor->unk60C == absent)
            {
                temp_v0_2 = (Record *)(item_record_offset + record_offset + (u8 *)record_base);
                temp_v0_2->bits.word = (u16)(temp_v0_2->bits.word & 0xFBFF);
                temp_v0_2->bits.byte.low = absent;
                temp_v0_2->unk0 = 0;
                temp_v0_2->unk4 = 0;
                temp_v0_2->unk6 = 0;
                temp_v0_2->bits.word = (u16)(temp_v0_2->bits.word & 0xFCFF);
            }
            else
            {
                temp_v1_6 = item_cursor->unk60C;
                if (temp_v1_6 & 0x80)
                {
                    temp_a3 = (Record *)(item_record_offset + record_offset + (u8 *)record_base);
                    temp_a3->unk0 = 0;
                    temp_a3->bits.word = (u16)(temp_a3->bits.word | 0x400);
                    temp_a0_4 = (Item *)(item_context + (context_offset + 0x5F0) + (((temp_v1_6 & 0x7F) << 6) + 0x150));
                    temp_a3->bits.byte.low = (s8)((u8)temp_a0_4->unk25 >> 1);
                    temp_a3->unk4 = (s16) * (temp_a0_4->unk24 + D_800EB24C);
                    temp_a1_3 = temp_a0_4->unk25;
                    texture_value = temp_a1_3 + 0x8018 + (temp_a0_4->unk24 * 0xE);
                    if (!(temp_a1_3 & 1))
                    {
                        texture_value += 0x800;
                    }
                    temp_a3->unk6 = texture_value;
                    temp_a3->bits.word = (u16)(temp_a3->bits.word & 0xFCFF);
                }
                else
                {
                    temp_a0_5 = (Record *)(item_record_offset + record_offset + (u8 *)record_base);
                    temp_a0_5->bits.word = (u16)(temp_a0_5->bits.word & 0xFBFF);
                    temp_a0_5->bits.byte.low = absent;
                    temp_a0_5->unk4 = 2;
                    temp_a0_5->unk0 = (s16)(item_cursor->unk60C | 0x8000);
                    temp_a0_5->bits.word = (u16)(temp_a0_5->bits.word & 0xFCFF);
                    temp_a0_5->unk6 = (s16)(((item_cursor->unk60C + 0x88) | ~0x7FFF) + (party->head.bytes.kind * 0x18));
                }
            }
            item_cursor = (Context *)((u8 *)item_cursor + 1);
            item_record_offset += 8;
        }
            if ((s32)item_cursor < (s32)((u8 *)temp_t4 + 4))
            {
                goto item_loop;
            }
            goto block_51;
        }
    }
    else
    {
    block_51:
        party++;
    }
    actor_stride_words += 0x14;
    record_stride += 0x190;
    context_stride += 0x250;
    player_index += 1;
    state_stride += 0x23C;
}
    if (player_index < 3)
    {
        goto party_loop;
    }
    func_8008C7A8();
}
