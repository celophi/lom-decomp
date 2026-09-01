#include "common.h"

typedef struct
{
    u8 pad0[0x18];
    s32 unk18;
} Struct_D80105880;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x244 - 0x25];
} FieldActorState;

typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[0x2A - 0x26];
    s16 unk2A;
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;
} Struct_D800FDF58;

extern Struct_D80105880 D_80105880[];
extern FieldActorState g_field_actor_slots[];
extern void func_80095074(Struct_D800FDF58 *rec);

/**
 * @brief Resets a field record when its selected actor slot is free.
 *
 * The record's selector at 0x3A chooses one of three D_80105880 entries, with
 * values >= 2 clamped to the third entry. If that entry's actor slot is free,
 * clears the record timer, writes the 0xFF sentinel, and calls func_80095074.
 *
 * @note The s32 return type, despite the lack of an explicit return statement,
 *       is required to preserve the target v0 lifetime. gcc272_cdk, 100%.
 */
s32 func_80094FDC(Struct_D800FDF58 *rec)
{
    FieldActorState *actors;
    u8 *base;
    s32 offset;
    s32 idx;
    FieldActorState *actor;

    actors = g_field_actor_slots;
    base = (u8 *)D_80105880;
    if (rec->unk3A < 2)
        offset = rec->unk3A * 0x1C;
    else
        offset = 0x38;
    idx = *(s32 *)(base + offset + 0x18);
    actor = actors + idx;
    if (actor->unk24 == 0)
    {
        rec->unk2A = 0;
        rec->unk25 = 0xFF;
        func_80095074(rec);
    }
}
