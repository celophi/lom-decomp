#include "common.h"

/** @brief Per-actor slot in D_80105AE0; stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} SlotA;

/** @brief Per-actor record in D_800FDF58; stride 0x54. */
typedef struct
{
    u8 pad0[0x21];
    u8 unk21;
    u8 pad22[0x3B - 0x22];
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} EntryB;

typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    u8 padA[0xE - 0xA];
    s16 unkE;
    u32 flags;
} FieldResourceEntry;

extern SlotA D_80105AE0[];
extern EntryB D_800FDF58[];
extern FieldResourceEntry g_field_resource_entries[];

/**
 * @brief Derive a control value for the actor slot matching the requested key.
 * @param arg0 Actor-slot lookup key.
 * @return Derived control value, or -1 when no matching actor slot exists.
 */
s32 func_8008B288(s32 arg0)
{
    EntryB* scan;
    EntryB* found;
    SlotA* e;
    s32 i;
    s32 result;
    s32 state;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == arg0)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (EntryB*)-1;
check:
    if (found != (EntryB*)-1)
    {
        goto lookup;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
lookup:
    if (g_field_resource_entries[found->unk3B].flags & 1)
    {
        goto special;
    }
    {
        state = found->unk21;
        if ((state & 0x7F) < 0xF)
        {
            switch (found->unk21)
            {
            case 1:
            case 6:
            case 11:
                result = 0x60;
                break;
            case 2:
            case 7:
            case 12:
                result = 0x80;
                break;
            case 3:
            case 8:
            case 13:
                result = 0xA0;
                break;
            case 4:
            case 9:
            case 14:
                result = 0xC0;
                break;
            case 129:
            case 134:
            case 139:
                result = 0x20;
                break;
            case 130:
            case 135:
            case 140:
                result = 0;
                break;
            case 131:
            case 136:
            case 141:
                result = 0xE0;
                break;
            case 132:
            case 137:
            case 142:
                result = 0xC0;
                break;
            case 0:
            case 5:
            case 10:
            case 128:
            case 133:
            case 138:
            default:
                result = 0x40;
                break;
            }
        }
        else
        {
            result = 0x40;
        }
    }
done:
    return result;
special:
    result = found->unk21 & 0x80;
    goto done;
}
