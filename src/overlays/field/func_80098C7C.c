#include "common.h"

typedef struct
{
    u8 pad0[0x20];
    u8 unk20;
    u8 unk21;
    u8 pad22[0x24 - 0x22];
    u8 unk24;
    u8 pad25[0x27 - 0x25];
    u8 unk27;
    u8 pad28[0x2A - 0x28];
    s16 unk2A;
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} Struct_D800FDF58;

typedef struct
{
    u8 *start;
    u8 *end;
    u8 unk8;
    u8 slot_index;
    u8 padA[0xE - 0xA];
    s16 unkE;
    u32 flags;
} FieldResourceEntry;

typedef struct
{
    u8 pad0[0x18];
    u16 unk18;
    u8 pad1A[0x23C - 0x1A];
} SlotA;

extern SlotA D_80105AE0[];
extern Struct_D800FDF58 D_800FDF58[];
extern FieldResourceEntry g_field_resource_entries[];

/**
 * @brief Initialize a field actor resource state when its slot is active.
 * @param arg0 Actor state record to update.
 * @param arg1 Slot index used to select the associated field tables.
 */
void func_80098C7C(Struct_D800FDF58 *arg0, s32 arg1)
{
    u8 *res;
    s32 idx8;
    SlotA *slot_base;
    SlotA *slot;

    if (arg0->unk3A != 0)
    {
        return;
    }
    idx8 = arg1 << 3;
    if (arg0->unk2A != 0)
    {
        return;
    }
    slot_base = D_80105AE0;
    slot = (SlotA *)((u8 *)slot_base + ((((idx8 + arg1) << 4) - arg1) << 2));
    if ((slot->unk18 & 1) == 0)
    {
        return;
    }

    func_80098FC4((Struct_D800FDF58 *)((arg1 * 0x54) + (s32)D_800FDF58), 0);
    arg0->unk2A = 0x95;
    arg0->unk20 = 0xA;

    if (g_field_resource_entries[arg0->unk3B].flags & 1)
    {
        arg0->unk21 &= 0x80;
    }
    else
    {
        u8 b = arg0->unk21;
        s32 masked = b & 0x7F;
        arg0->unk21 = (masked % 5) | (b & 0x80);
    }

    arg0->unk27 = 0;
    arg0->unk24 = 1;
    res = g_field_resource_entries[arg0->unk3B].start;
    func_8006C3FC(arg0, res);
}
