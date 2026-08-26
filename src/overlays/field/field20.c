#include "common.h"

typedef struct
{
    u8 pad0[0x228];
    u8 unk228;
    u8 pad229[0x23A - 0x229];
    u8 unk23A;
    u8 pad23B[0x244 - 0x23B];
} FieldActorState;

extern FieldActorState g_field_actor_slots[];

/**
 * @brief Test whether any active actor slot matches the given id and is flagged.
 * @param arg0 Actor id to look for (compared against slot field unk228).
 * @return 1 if a matching slot with unk23A set is found, else 0.
 */
s32 func_80083AB4(s32 arg0)
{
    s32 i;

    i = 0;
    do
    {
        if ((g_field_actor_slots[i].unk228 == arg0) && (g_field_actor_slots[i].unk23A != 0))
        {
            return 1;
        }
        i++;
    } while (i < 0x30);
    return 0;
}
