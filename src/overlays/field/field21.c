#include "common.h"

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
} UnkRecord;

typedef struct
{
    u8 pad0[0x228];
    u8 unk228;
    u8 pad229[0x244 - 0x229];
} FieldActorState;

extern FieldActorState g_field_actor_slots[];

void func_80083BC0(UnkRecord *arg0, FieldActorState *actor, s32 arg1);
void func_800A3B78(s32 arg0);

/**
 * @brief Dispatch a record against every actor slot whose id matches it, then
 *        run the record's post-pass handler.
 * @param arg0 Source record; its unk3A field is the actor id to match.
 * @param arg1 Opaque parameter forwarded to func_80083BC0.
 */
void func_80083B38(UnkRecord *arg0, s32 arg1)
{
    FieldActorState *actor;
    s32 i;

    actor = g_field_actor_slots;
    for (i = 0; i < 0x30; i++, actor++)
    {
        if (actor->unk228 == arg0->unk3A)
        {
            func_80083BC0(arg0, actor, arg1);
        }
    }
    func_800A3B78(arg0->unk3A);
}
