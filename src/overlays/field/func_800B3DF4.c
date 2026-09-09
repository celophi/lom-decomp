#include "common.h"

/** @brief Partial HeaderB800B3DF4 layout used by func_800B3DF4. */
typedef struct
{
    u8 pad0[0x400];
    u16 unk400;
} HeaderB800B3DF4;

/** @brief Partial SlotB800B3DF4 layout used by func_800B3DF4. */
typedef struct
{
    u8 pad0[0x430];
    u8 unk430;
    u8 pad431[0x4C0 - 0x431];
    u32 unk4C0;
} SlotB800B3DF4;

extern u8 *D_80122B78;
extern u8 *D_80123FB0;

void akao_set_song_params(s32, s32, s32, s32);

s32 func_80087F0C(s32 arg0);
void func_800B3F1C(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Build field records for actors whose status nibble matches the requested key.
 * @param arg0 Status key to match.
 * @return Number of matching actor records processed.
 * @note WIP: frame, allocation and control-flow differences remain.
 */
s32 func_800B3DF4(s32 arg0)
{
    s32 i;
    s32 count;
    s32 slot_off;
    s32 rec_off;
    s32 result;

    count = 0;
    slot_off = 0x1BC;
    rec_off = 0x160;
    for (i = 3; i < (s32) ((HeaderB800B3DF4 *) D_80122B78)->unk400; i++, slot_off += 0x94)
    {
        SlotB800B3DF4 *slot = (SlotB800B3DF4 *) (D_80122B78 + slot_off);
        if ((slot->unk4C0 & 0xF) == arg0)
        {
            result = func_80087F0C(slot->unk430);
            if (result == 0)
            {
                goto do_call;
            }
            if (result != -1)
            {
                goto skip_call;
            }
        do_call:
            akao_set_song_params(0x8001, 0x64, arg0, ((SlotB800B3DF4 *) (D_80122B78 + slot_off))->unk430);
        skip_call:
            count += 1;
            func_800B3F1C(((SlotB800B3DF4 *) (D_80122B78 + slot_off))->unk430, (s32) (D_80123FB0 + rec_off), result);
            rec_off += 0x68;
        }
    }
    return count;
}
