#include "common.h"

/** @brief Per-actor slot in D_80105AE0; stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} FieldActorSlot;

/** @brief Per-actor record in D_800FDF58; stride 0x54. */
typedef struct
{
    u8 pad0[0x20];
    u8 unk20;
    u8 unk21;
    u8 pad22[0x2A - 0x22];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} FieldActorRecord;

extern FieldActorSlot D_80105AE0[];
extern FieldActorRecord D_800FDF58[];

/**
 * @brief Marks the actor record matching @p key as busy and derives its next control state.
 * @param key Value compared against each slot's unk14.
 * @return 0 when a matching slot was found and updated, -1 otherwise.
 */
s32 func_80089980(s32 key)
{
    FieldActorSlot* e;
    FieldActorRecord* scan;
    FieldActorRecord* found;
    s32 i;
    s32 result;
    u8 state;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (FieldActorRecord*)-1;
check:
    if (found != (FieldActorRecord*)-1)
    {
        goto body;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
body:
    found->unk2A = 0xB2;
    state = found->unk21;
    if (state >= 0x8F)
    {
        goto set_default;
    }
    switch (state)
    {
    case 1:
    case 6:
    case 11:
        found->unk20 = 6;
        break;
    case 2:
    case 7:
    case 12:
        found->unk20 = 0xC;
        break;
    case 3:
    case 8:
    case 13:
        found->unk20 = 0x12;
        break;
    case 4:
    case 9:
    case 14:
    case 132:
    case 137:
    case 142:
        found->unk20 = 0x18;
        break;
    case 131:
    case 136:
    case 141:
        found->unk20 = 0x1E;
        break;
    case 130:
    case 135:
    case 140:
        found->unk20 = 0x24;
        break;
    case 129:
    case 134:
    case 139:
        found->unk20 = 0x2A;
        break;
    case 0:
        set_default:
        found->unk20 = 0;
        break;
    }
    result = 0;
done:
    return result;
}
