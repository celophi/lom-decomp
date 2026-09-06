#include "common.h"

typedef struct
{
    u8 pad4[4];
    s32 unk4;
} SubState;

typedef struct
{
    s32 unk0;
} EntryB800B5948;

typedef struct
{
    u32 unk0;
    u8 pad4[0x14 - 4];
    s32 unk14;
    SubState *unk18;
    EntryB800B5948 *unk1C;
    s32 unk20;
    s32 unk24;
    u8 pad28[0x4A0 - 0x28];
    u16 unk4A0;
} FieldStateB800B5948;

typedef struct
{
    s32 unk0;
    u8 pad4[0xC - 4];
    s32 unkC;
} ArgB800B5948;

extern FieldStateB800B5948 *D_80123FB0;

extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);
s32 func_800B2A9C(s32 arg0);
s32 func_800B302C(s32 arg0, s32 arg1);
s32 func_800B4CE4(s32 arg0, s32 arg1);
EntryB800B5948 *func_800B50B8(s32 arg0, void *arg1);

/**
 * @brief Update the shared field state for a newly selected record.
 * @param arg0 Record used to initialize the shared state, or NULL to reset it.
 * @param arg1 Whether to resolve and update the associated entry.
 */
void func_800B5948(ArgB800B5948 *arg0, s32 arg1)
{
    EntryB800B5948 *entry;
    s32 val;

    D_80123FB0->unk18 = (SubState *) arg0;
    if (arg0 == NULL)
    {
        akao_set_song_params(0x8001, (s32) func_800B5948, 0, 0);
        return;
    }
    D_80123FB0->unk20 = func_800B2A9C(arg0->unk0);
    D_80123FB0->unk24 = func_800B2A9C(arg0->unkC);
    D_80123FB0->unk14 = 0;
    D_80123FB0->unk14 = (D_80123FB0->unk14 & ~2) | ((func_800B302C(arg0->unk0, arg0->unkC) & 1) * 2);
    if (arg1 != 0)
    {
        D_80123FB0->unk1C = func_800B50B8(-3, D_80123FB0);
        if (func_800B4CE4(D_80123FB0->unk20, 4) != 0)
        {
            entry = D_80123FB0->unk1C;
            val = entry->unk0;
            if ((u32) (val & 0xF) < 2U)
            {
                entry->unk0 = val | 0xC0;
                D_80123FB0->unk4A0 = (u16) ((D_80123FB0->unk4A0 * 3) >> 1);
            }
        }
    }
    else
    {
        D_80123FB0->unk1C = NULL;
    }
}
