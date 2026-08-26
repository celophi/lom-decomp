#include "common.h"

typedef struct
{
    u32 unk0;
    u8 pad4[0x18 - 4];
    s32 unk18;
} Struct_D80105880;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x244 - 0x25];
} FieldActorState;

extern Struct_D80105880 D_80105880[];
extern FieldActorState g_field_actor_slots[];

/**
 * @brief Find a free field actor slot not already claimed by one of the three
 *        active tracks.
 * @param arg0 Track index (clamped to 2 when >= 3).
 * @param arg1 When non-zero, require the (clamped) track's D_80105880 entry to
 *             be idle; otherwise fail early.
 * @return Index of the first free, unclaimed actor slot, or -1 if none.
 */
s32 func_800839F8(s32 arg0, s32 arg1)
{
    s32 i;
    s32 j;
    Struct_D80105880 *entry;

    if (arg1 != 0)
    {
        entry = D_80105880;
        if (arg0 >= 3)
        {
            arg0 = 2;
        }
        if (entry[arg0].unk0 != 0)
        {
            return -1;
        }
    }

    for (i = 0; i < 0x30; i++)
    {
        if (g_field_actor_slots[i].unk24 == 0)
        {
            for (j = 0; j < 3; j++)
            {
                if (D_80105880[j].unk0 != 0 && D_80105880[j].unk18 == i)
                {
                    break;
                }
            }
            if (j == 3)
            {
                return i;
            }
        }
    }
    return -1;
}
