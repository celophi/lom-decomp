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

/** @brief View of the D_80122B74 block: a byte at 0x2E5 and 0xC-byte rows at 0x2F4. */
typedef struct
{
    u8 pad[0x2E5];
    u8 unk2E5;
    u8 pad2E6[0x2F4 - 0x2E6];
    u8 unk2F4[1][0xC];
} StructB74;

#define FIELD_B74 ((StructB74 *)D_80122B74)

void akao_set_song_params(s32, s32, s32, s32);
void func_800BD520(s32, s32, s32);
s32 func_800B37D4(void);
s32 func_800B3DF4(s32);
void func_800B4390(void);
void func_800C1EC8(s32, void *, s32);
u8 *func_800C1E40(s32);
u32 func_800BD414(s32, s32);
s32 func_800C3688(s32);
void func_800B3580(void);
s32 func_800B3670(s32);

extern u8 *D_80122B74;
extern s32 D_8010D020;
extern u8 D_800EF8C0[];
extern u8 D_800F0B48[];
extern u8 D_800F0AE8[];
extern StructB3580 D_80123B08;
extern u8 *D_80123FAC;
extern StructB3580 *D_80123FB0;
extern u16 g_music_track_index;

/**
 * @brief Rebuild the D_80123B08 block and write script variables 0x4280 and 0x4284, or call func_800B4390 when arg0 is 0.
 *
 * With D_8010D020 set both variables are written as 1 instead of the
 * computed values. 0x4280 and 0x4284 are the counter pair that func_800B48B8
 * increments and func_800B62D8 tests for zero.
 *
 * @param arg0 Nonzero selects the rebuild path and is forwarded to func_800B3DF4.
 * @see decomp.me (100%)
 */
void func_800B34D0(s32 arg0)
{
    s32 value;

    if (arg0 != 0)
    {
        func_800B3580();
        value = func_800B37D4();
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4280, 1);
        }
        else
        {
            func_800BD520(0, 0x4280, value);
        }
        value = func_800B3DF4(arg0);
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4284, 1);
        }
        else
        {
            func_800BD520(0, 0x4284, value);
        }
    }
    else
    {
        func_800B4390();
    }
}

/**
 * @brief Zero the D_80123B08 block, then fill it from the current track's 0xC-byte layout record and resource 1.
 * @see decomp.me (100%)
 */
void func_800B3580(void)
{
    s32 i;
    u8 *p;

    D_80123FAC = D_800EF8C0;
    D_80123FB0 = &D_80123B08;
    func_800C1EC8(0, &D_80123B08, 0x4A4);
    D_80123FB0->unk18 = 0;
    D_80123FB0->unk0 = func_800B3670(0);

    for (i = 0; i < 8; i++)
    {
        D_80123FB0->unkC[i] = D_800F0B48[FIELD_B74->unk2F4[g_music_track_index][i]];
    }

    p = func_800C1E40(1);
    D_80123FB0->unk4 = p + *(s32 *)(p + 4);
    D_80123FB0->unk8 = p + *(s32 *)(p + 8);
    func_800BD520(0, 0x428C, -1);
}

/**
 * @brief Look up D_800F0AE8 by a 0..0x3F index and clamp the result to script variables 0x52E0..0x52E8 and 0x63.
 *
 * The index comes from the byte at 0x2E5 of the layout buffer when @p arg0 or
 * bit 7 of script variable 0x52F0 is set, otherwise from func_800C3688 for the
 * current track. Script variable 0x2938 adds 0x14 (mode 1) or forces 0x3F
 * (mode 2).
 *
 * @param arg0 Nonzero selects the byte-at-0x2E5 index.
 * @return Value in 0..0x63.
 * @see decomp.me (100%)
 */
s32 func_800B3670(s32 arg0)
{
    s32 flag;
    s32 mode;
    s32 index;
    s32 value;
    u32 lo;
    u32 hi;

    flag = arg0;
    if (func_800BD414(0, 0x52F0) & 0x80)
    {
        flag = 1;
    }
    mode = func_800BD414(0, 0x2938);

    if (flag != 0)
    {
        switch (mode)
        {
            case 1:
                index = FIELD_B74->unk2E5 + 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
            default:
                index = FIELD_B74->unk2E5;
                break;
        }
        index = (index * 3) / 2;
    }
    else
    {
        index = func_800C3688(g_music_track_index);
        switch (mode)
        {
            case 1:
                index += 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
        }
    }

    if (index >= 0x40)
    {
        index = 0x3F;
    }

    value = D_800F0AE8[index];
    lo = func_800BD414(0, 0x52E0);
    hi = func_800BD414(0, 0x52E8);
    if (value < lo)
    {
        value = lo;
    }
    else if (value > hi)
    {
        value = hi;
    }

    if (value >= 0x64)
    {
        value = 0x63;
    }
    return value;
}



extern u8 *D_80122B74;
extern s32 D_8010D020;
extern u8 *func_80087F0C(s32);
extern void func_800B3D84(void);
extern void func_800B4934(u8 *);
extern s32 func_800B7EE8(u8 *, s32);

typedef struct
{
    u8 pad0[0xC];
    u8 base_attributes[8];
    u8 pad14[0x14];
    u8 flags;
    u8 pad29[2];
    u8 unk2B;
    union
    {
        u32 word;
        struct
        {
            u8 index;
            u8 flags;
            u16 upper;
        } parts;
    } config;
    u8 unk30;
    u8 unk31;
    u16 unk32;
    u32 unk34;
    u8 *actor;
    u32 unk3C;
    u16 unk40;
    u8 unk42;
    u8 pad43;
    u16 equipment_stats[4];
    u8 equipment_attributes[4];
    u8 attributes[8];
    u8 base_values[8];
    u8 flags60;
    u8 flags61;
    u8 flags62;
    u8 pad63;
    u8 modifiers[8];
    u8 bonuses[8];
    u8 unk74;
} PartyActorView;
typedef struct
{
    u8 pad0[0x5F0];
    u8 active;
    u8 pad5F1[0x17];
    u8 type;
    u8 pad609[0xB];
    u16 hp;
    u8 pad616[0x1D];
    u8 unk633;
    u8 pad634[0xC];
    u8 equipment_active;
    u8 pad641[0x13];
    u32 equipment_config;
    u32 equipment_modifiers;
    u8 pad65C[8];
    u16 equipment_stat;
    u8 pad666[6];
    u8 flags66C;
    u8 flags66D;
    u8 pad66E[2];
    u8 equipment_attribute;
} PartySaveView;
typedef struct
{
    u8 pad0[0x24];
    u16 stat;
    u8 pad26[0xA];
    u8 attribute;
} PartyEquipmentView;
typedef struct
{
    s32 hp;
    s32 max_hp;
    u32 flags;
    u8 padC[0x5C];
    u16 capacity;
} PartyLiveActorView;

/**
 * @brief Initialize the three party actor records and their derived attributes.
 * @see decomp.me (100%)
 */
s32 func_800B37D4(void)
{
    s32 active_count;
    s32 equipment_address;
    s32 attribute_offset;
    s32 active_flags;
    s32 config_flags;
    u32 actor_type;
    s32 index;
    s32 stat_index;
    s32 equipment_index;
    s32 player_control;
    s8 attribute_value;
    s32 party_index;
    s8 actor_flags;
    u32 capacity;
    u32 packed_modifiers;
    u8 *config_record;
    u8 *linked_record;
    u8 *modifier_record;
    u8 *flags60_record;
    u8 *flags62_record;
    u8 *live_flags;
    u8 *setup_record;
    u8 *stat_record;
    u8 *hp_source;
    u8 *capacity_record;
    u8 *attribute_record;
    u8 *hp_record;
    u8 *live_hp;
    u8 *actor;
    u8 *value_record;
    u8 *final_record;
    u8 *live_capacity;
    u8 *flags_record;
    party_index = 0;
    active_count = 0;
    do
    {
        if (((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->active != 0)
        {
            if ((D_8010D020 != 0) && (party_index == 0))
            {
                flags_record = (u8 *)D_80123FB0;
                actor_flags = ((PartyActorView *)flags_record)->flags | 0x40;
            }
            else
            {
                flags_record = (u8 *)D_80123FB0;
                flags_record += party_index * 0x68;
                actor_flags = ((PartyActorView *)flags_record)->flags | 0x80;
            }
            ((PartyActorView *)flags_record)->flags = actor_flags;
            ((PartyActorView *)(((u8 *)D_80123FB0) + ((party_index * 0xD) << 3)))->unk2B = 0xF;
            ((PartyActorView *)(((u8 *)D_80123FB0) + ((party_index * 0xD) << 3)))->config.parts.index =
                party_index;
            config_record = ((u8 *)D_80123FB0) + (party_index * 0x68);
            config_flags = ((PartyActorView *)config_record)->config.word;
            active_flags = config_flags | 0x100;
            ((PartyActorView *)config_record)->config.word = active_flags;
            if (D_8010D020 != 0)
            {
                player_control = 0;
                if (party_index == 0)
                {
                    player_control = 0xFF;
                }
                ((PartyActorView *)config_record)->config.word =
                    (s32)((active_flags & (~0x200)) | ((player_control & 1) << 9));
            }
            else
            {
                ((PartyActorView *)config_record)->config.word = (s32)(config_flags | 0x300);
            }
            setup_record = ((u8 *)D_80123FB0) + (party_index * 0x68);
            actor_type = ((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->type;
            ((PartyActorView *)setup_record)->config.word =
                (s32)((((PartyActorView *)setup_record)->config.word & 0xFFFF03FF) |
                      ((actor_type & 0x3F) << 10));
            ((PartyActorView *)setup_record)->unk30 = 5;
            ((PartyActorView *)setup_record)->config.parts.upper = 0;
            ((PartyActorView *)(((u8 *)D_80123FB0) + ((party_index * 0xD) << 3)))->unk31 = 5;
            actor = ((u8 *)D_80123FB0) + (party_index * 0x68);
            ((PartyActorView *)actor)->unk32 = 0;
            ((PartyActorView *)actor)->unk34 = 0;
            actor = func_80087F0C(party_index);
            stat_index = 0;
            linked_record = ((u8 *)D_80123FB0) + (party_index * 0x68);
            ((PartyActorView *)linked_record)->actor = actor;
            ((PartyActorView *)linked_record)->unk40 =
                (u16)((PartySaveView *)(((party_index * 0x25) << 4) + ((s32)D_80122B74)))->equipment_stat;
            ((PartyActorView *)linked_record)->unk42 = 0x19;
            do
            {
                index = 1;
                ((PartyActorView *)(((u8 *)D_80123FB0) + (((party_index * 0xD) << 3) + (stat_index << 1))))
                    ->equipment_stats[0] = 0;
                ((PartyActorView *)(((u8 *)D_80123FB0) + (((party_index * 0xD) << 3) + stat_index)))
                    ->equipment_attributes[0] =
                    (u8)((PartySaveView *)((((party_index * 0x25) << 4) + ((s32)D_80122B74)) + stat_index))
                        ->equipment_attribute;
                do
                {
                    if (((PartySaveView *)(D_80122B74 + (((party_index * 0x25) << 4) + (index * 0x40))))
                            ->equipment_active != 0)
                    {
                        stat_record = ((u8 *)D_80123FB0) + ((stat_index << 1) + (party_index * 0x68));
                        attribute_record = ((u8 *)D_80123FB0) + ((party_index * 0x68) + stat_index);
                        equipment_address =
                            (s32)(((((party_index * 0x25) << 4) + ((s32)D_80122B74)) + (index << 6)) + 0x640);
                        ((PartyActorView *)stat_record)->equipment_stats[0] =
                            (u16)(((PartyActorView *)stat_record)->equipment_stats[0] +
                                  ((PartyEquipmentView *)(equipment_address + (stat_index << 1)))->stat);
                        ((PartyActorView *)attribute_record)->equipment_attributes[0] =
                            (u8)(((PartyActorView *)attribute_record)->equipment_attributes[0] +
                                 ((PartyEquipmentView *)(equipment_address + stat_index))->attribute);
                    }
                    index += 1;
                } while (index < 4);
                stat_index += 1;
            } while (stat_index < 4);
            index = 0;
            func_800B4934(((u8 *)D_80123FB0) + ((party_index * 0x68) + 0x28));
            packed_modifiers =
                ((PartySaveView *)(((party_index * 0x25) << 4) + ((s32)D_80122B74)))->equipment_modifiers;
            do
            {
                attribute_value = func_800B7EE8(D_80122B74 + (((party_index * 0x25) << 4) + 0x5F0), index);
                attribute_offset = index + ((party_index * 0xD) << 3);
                value_record = ((u8 *)D_80123FB0) + attribute_offset;
                ((PartyActorView *)value_record)->base_values[0] = attribute_value;
                ((PartyActorView *)value_record)->attributes[0] = attribute_value;
                ((PartyActorView *)(((u8 *)D_80123FB0) + attribute_offset))->modifiers[0] =
                    (s8)(packed_modifiers & 0xF);
                packed_modifiers = packed_modifiers >> 4;
                modifier_record = ((u8 *)D_80123FB0) + attribute_offset;
                ((PartyActorView *)modifier_record)->modifiers[0] +=
                    ((PartyActorView *)(((u8 *)D_80123FB0) + index))->base_attributes[0];
                index += 1;
            } while (index < 8);
            equipment_index = 1;
            ((PartyActorView *)(((u8 *)D_80123FB0) + (index + ((party_index * 0xD) << 3))))->bonuses[0] = 0;
            ((PartyActorView *)(((u8 *)D_80123FB0) + ((party_index * 0xD) << 3)))->flags60 = 0;
            ((PartyActorView *)(((u8 *)D_80123FB0) + ((party_index * 0xD) << 3)))->flags61 = 0;
            ((PartyActorView *)(((u8 *)D_80123FB0) + ((party_index * 0xD) << 3)))->flags62 = 0;
            do
            {
                if (((PartySaveView *)(D_80122B74 + (((party_index * 0x25) << 4) + (equipment_index << 6))))
                        ->equipment_active != 0)
                {
                    packed_modifiers = ((PartySaveView *)((((party_index * 0x25) << 4) + ((s32)D_80122B74)) +
                                                          (equipment_index << 6)))
                                           ->equipment_modifiers;
                    for (index = 0; index < 8; index++)
                    {
                        ((PartyActorView *)(((u8 *)D_80123FB0) - (-(index + (party_index * 0x68)))))
                            ->bonuses[0] += packed_modifiers & 0xF;
                        packed_modifiers >>= 4;
                    }

                    flags60_record = ((u8 *)D_80123FB0) + (party_index * 0x68);
                    ((PartyActorView *)flags60_record)->flags60 =
                        (u8)(((PartyActorView *)flags60_record)->flags60 |
                             ((PartySaveView *)((((party_index * 0x25) << 4) + ((s32)D_80122B74)) +
                                                (equipment_index << 6)))
                                 ->flags66C);
                    flags62_record = ((u8 *)D_80123FB0) + (party_index * 0x68);
                    ((PartyActorView *)flags62_record)->flags62 =
                        (u8)(((PartyActorView *)flags62_record)->flags62 |
                             ((PartySaveView *)((((party_index * 0x25) << 4) + ((s32)D_80122B74)) +
                                                (equipment_index << 6)))
                                 ->flags66D);
                }
                equipment_index += 1;
            } while (equipment_index < 4);
            final_record = ((u8 *)D_80123FB0) + (party_index * 0x68);
            ((PartyActorView *)final_record)->unk3C = 0;
            ((PartyActorView *)final_record)->unk74 =
                (u8)((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->unk633;
            if (D_8010D020 != 0)
            {
                hp_source = D_80122B74 + ((party_index * 0x25) << 4);
                hp_record = ((u8 *)D_80123FB0) + (party_index * 0x68);
                *((s32 *)((PartyActorView *)hp_record)->actor) = (s32)(((PartySaveView *)hp_source)->hp * 3);
                ((PartyLiveActorView *)((PartyActorView *)hp_record)->actor)->max_hp =
                    (s32)(((PartySaveView *)hp_source)->hp * 3);
                live_hp = ((PartyActorView *)hp_record)->actor;
                ((PartyLiveActorView *)live_hp)->flags =
                    (s32)((((PartyLiveActorView *)live_hp)->flags & 0xFF000000) |
                          (((PartySaveView *)hp_source)->hp * 3));
                live_flags = ((PartyActorView *)hp_record)->actor;
                ((PartyLiveActorView *)live_flags)->flags =
                    (s32)(((PartyLiveActorView *)live_flags)->flags & 0x80FFFFFF);
            }
            else
            {
                *((s32 *)((PartyActorView *)(((u8 *)D_80123FB0) + ((party_index * 0xD) << 3)))->actor) =
                    (s32)((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->hp;
            }
            capacity_record = ((u8 *)D_80123FB0) + (party_index * 0x68);
            capacity = ((PartyActorView *)capacity_record)->attributes[3] * 2;
            if (((((u32)((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->equipment_config) >>
                  0xA) &
                 0x3F) == 7)
            {
                capacity += 0x80;
            }
            live_capacity = ((PartyActorView *)capacity_record)->actor;
            if (capacity < 0x100U)
            {
                ((PartyLiveActorView *)live_capacity)->capacity = capacity;
            }
            else
            {
                ((PartyLiveActorView *)live_capacity)->capacity = 0xFFU;
            }
            if ((((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->type & 0x7F) == 3)
            {
                func_800B3D84();
            }
            active_count += 1;
        }
        party_index += 1;
    } while (party_index < 3);
    return active_count;
}


extern u8 *D_80122B74;
/* No prototype is in scope at the original call site, so the arguments pass as
 * plain int (the target does not truncate the field to s16). */
void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);


/**
 * @brief Restarts field music when the active scene index is out of range.
 *
 * Reads the active scene index at offset 0x2EF0 of the D_80122B74 buffer; if it
 * is 5 or greater it re-arms akao_set_song_params, then forwards the scene
 * entry's 0x2F3C word (stride 0x60) to func_800BD520.
 *
 * Matches under GCC 2.8.0. The pre-diagnostic scene index and the index
 * reloaded afterward are distinct value webs; materializing the second
 * index's 0x60-byte offset reproduces the target allocation exactly.
 */
void func_800B3D84(void)
{
    s32 idx1;
    s32 idx2;
    s32 off;

    idx1 = *(s32 *)(D_80122B74 + 0x2EF0);
    if ((u32)idx1 >= 5)
    {
        akao_set_song_params(0x8001, 0x75, idx1, 0);
    }

    idx2 = *(s32 *)(D_80122B74 + 0x2EF0);
    off = idx2 * 0x60;
    func_800BD520(2, 0xF020, *(s32 *)(D_80122B74 + off + 0x2F3C));
}
