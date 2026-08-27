#include "common.h"

typedef struct
{
    u8 pad0[0x24];
    u8 unk24; /* 0x24 */
    u8 pad25[0x244 - 0x25];
} FieldActorState;

typedef struct
{
    u8 pad0[0x25];
    u8 unk25; /* 0x25 */
    u8 pad26[0x2A - 0x26];
    s16 unk2A; /* 0x2A */
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A; /* 0x3A */
    u8 pad3B[0x54 - 0x3B];
} Struct_D800FDF58;

extern FieldActorState g_field_actor_slots[];
extern void func_80095074(Struct_D800FDF58 *rec);

/**
 * @brief Resets an actor record when its target field-actor slot is free.
 *
 * When the slot at @c g_field_actor_slots[rec->unk3A + 0x40] has a zero
 * @c unk24 (unclaimed), clears the record's @c unk2A, sets @c unk25 to the
 * sentinel 0xFF, and hands the record to func_80095074.
 *
 * @param rec Actor record to reset.
 * @return Unused status value (the return register is left live by the
 *         original, but no caller consumes it).
 */
s32 func_80095168(Struct_D800FDF58 *rec)
{
    if (g_field_actor_slots[rec->unk3A + 0x40].unk24 == 0)
    {
        rec->unk2A = 0;
        rec->unk25 = 0xFF;
        func_80095074(rec);
    }
}
