#include "common.h"

/** @brief Partial Rec layout used by func_800C0A38. */
typedef struct Rec
{
    u8 pad0[0xC];
    s32 unkC;
    u8 *unk10;
    u8 *unk14;
} Rec;

extern u8 D_800F18C4[];
extern s32 (*D_800F18B4[])(s32, s32, Rec *);

s32 rand(void);

/**
 * @brief Select a bounded random entry and invoke its dispatch handler.
 * @param arg0 Actor record containing flags and selection tables.
 * @return The handler result, or -1 for a null record or invalid dispatch entry.
 * @note WIP: load scheduling and temporary-register differences remain.
 * @see decomp.me (91.29%) WIP
 */
s32 func_800C0A38(Rec *arg0)
{
    s32 result;
    s32 count;
    s32 mask;
    s32 shift_count;
    s32 within_range;
    u8 *entry;
    u8 dispatch_idx;

    result = -1;
    if (arg0 != NULL)
    {
        count = D_800F18C4[(u8)(arg0->unk10[0x4C]) >> 5];
        within_range = count < 8;
        if (arg0->unkC & 0x04000000)
        {
            count += 2;
            within_range = count < 8;
        }
        if (!within_range)
        {
            count = 7;
        }
        shift_count = rand();
        mask = shift_count & 0xFFFF;
        if (arg0->unkC & 0x08000000)
        {
            mask = shift_count & 0xFFFC;
        }
        shift_count = 0;
        if (count != 0)
        {
            while (!(mask & 1))
            {
                shift_count += 1;
                mask >>= 1;
                if (shift_count >= count)
                {
                    break;
                }
            }
        }
        entry = arg0->unk14 + (shift_count * 2);
        if (*(volatile u8 *)(entry + 0x40) >= 4)
        {
            return -1;
        }
        dispatch_idx = entry[0x40];
        result = D_800F18B4[dispatch_idx](dispatch_idx, entry[0x41], arg0);
    }
    return result;
}

extern u8 *D_80123FB0;
void func_800C0E54(s32, s32);
void func_800C0E18(s32, s32);
void func_800C1A18(void *, void *);
void func_800C1B20(s32, s32);
/** @brief Dispatches an experience, currency, item or counter reward.
 * @note Initial nonmatching C recovered with the seven-entry jump table.
 */
void func_800C0B40(s32 recipient, void *arg1, u32 arg2)
{
    s32 var_a1;
    u8 var_v1;
    u8 var_v1_2;

    switch (arg2)

    {
    case 0:
        var_v1 = *D_80123FB0;
        var_a1 = var_v1 * 4;
        if ((s32) var_v1 >= 0xB)
        {
            var_v1 = ((s32) (var_v1 - 0xA) / 2) + 0xA;
            var_a1 = var_v1 * 4;
        }
        func_800C0E54(recipient, (var_a1 + var_v1) * 2);
        return;
    case 1:
        var_v1_2 = *D_80123FB0;
        if ((s32) var_v1_2 >= 0xB)
        {
            var_v1_2 = ((s32) (var_v1_2 - 0xA) / 2) + 0xA;
        }
        func_800C0E54(recipient, var_v1_2);
        return;
    case 2:
        func_800C0E18(recipient, 0x32);
        return;
    case 3:
        func_800C0E18(recipient, 0xA);
        return;
    case 4:
        func_800C1A18((void *)recipient, arg1);
        return;
    case 5:
        func_800C1B20(recipient, 0x40);
        return;
    case 6:
        func_800C1B20(recipient, 0x80);
        return;
    default:
        akao_set_song_params(0x8001, 0x12C, (s32)arg1, arg2);
        return;
    }
}


typedef struct Quad
{
    u8 pad0[0x60];
    u8 unk60;
    u8 unk61;
    u8 unk62;
    u8 unk63;
} Quad;

typedef struct Flag
{
    u8 pad0[0x3F];
    u8 unk3F;
} Flag;

typedef struct DirectionalRec
{
    u8 pad0[0xC];
    s32 unkC;
    Quad *unk10;
    Flag *unk14;
} DirectionalRec;

/**
 * @brief Decode packed directional flags into the record output bytes.
 * @param arg0 Unused context pointer.
 * @param arg1 Packed directional value.
 * @param arg2 Record receiving the decoded values.
 * @return Constant command length value 0x1F.
 */
s32 func_800C0C74(void *arg0, s32 arg1, DirectionalRec *arg2)
{
    s32 temp_a0;
    s32 var_v1;

    var_v1 = arg1 >> 4;
    temp_a0 = arg2->unkC;
    arg1 = arg1 & 0xF;
    if (temp_a0 < 0)
    {
        arg1 += var_v1;
        var_v1 = 0;
    }
    if (temp_a0 & 0x20000000)
    {
        arg1 += 2;
    }
    if (temp_a0 & 0x10000000)
    {
        arg1 += 1;
    }
    if (temp_a0 & 0x02000000)
    {
        var_v1 += 2;
    }
    if (temp_a0 & 0x01000000)
    {
        var_v1 += 1;
    }
    if (arg2->unk14->unk3F & 0x80)
    {
        arg2->unk10->unk60 = (s8) var_v1;
        arg2->unk10->unk61 = 0;
        arg2->unk10->unk62 = (s8) arg1;
        arg2->unk10->unk63 = 0;
    }
    else
    {
        arg2->unk10->unk60 = 0;
        arg2->unk10->unk61 = (s8) var_v1;
        arg2->unk10->unk62 = 0;
        arg2->unk10->unk63 = (s8) arg1;
    }

    return 0x1F;
}


typedef struct
{
    u8 unk0[4];
    u8 unk4;
} UnkStruct800C0D90_Arg2;

typedef struct
{
    u8 unk0[2];
    s16 unk2;
} UnkStruct800C0D90_Ret;

typedef struct
{
    u8 unk0[0x18];
    u8 unk18;
} UnkStruct800C0DC4_Ptr;

typedef struct
{
    u8 unk0[4];
    u8 unk4;
    u8 unk5[0xF];
    UnkStruct800C0DC4_Ptr *unk14;
} UnkStruct800C0DC4_Arg2;

s32 func_800C0D58(s32 arg0, s32 arg1)
{
    if ((rand() & 0xFF) < arg1)
    {
        return 0x22;
    }

    return 0x21;
}

extern UnkStruct800C0D90_Ret *func_800C1B60(u8 arg0);

s32 func_800C0D90(s32 arg0, s32 arg1, UnkStruct800C0D90_Arg2 *arg2)
{
    func_800C1B60(arg2->unk4)->unk2 = (s16) (arg1 | 0x8000);
    return 0x20;
}



s32 func_800C0DC4(s32 arg0, s32 arg1, UnkStruct800C0DC4_Arg2 *arg2)
{
    func_800C1B60(arg2->unk4)->unk2 = (s16) (arg1 + (arg2->unk14->unk18 << 4));
    return 0x20;
}
