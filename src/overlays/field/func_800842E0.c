#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 pad8[0xC - 8];
    s32 unkC;
    u8 pad10[0x18 - 0x10];
    s32 unk18;
} Struct_D80105880;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 unk25;
    u8 pad26[0x224 - 0x26];
    s32 unk224;
    u8 unk228;
    u8 pad229[0x244 - 0x229];
} FieldActorState;

extern Struct_D80105880 D_80105880[];
extern FieldActorState g_field_actor_slots[];

s32 func_8009A390(void);
void func_8009A3E8(void);
void func_8009A4CC(s32 arg0, void *arg1);
void func_80084424(s32 arg0);
s32 cdrom_can_queue_resource(s32 resource_index);

/**
 * @brief Retire completed resource-load slots and finish processing when all slots are idle.
 */
void func_800842E0(void)
{
    s32 i;
    s32 idle_count;
    Struct_D80105880 *entry;
    FieldActorState *actor;

    entry = D_80105880;
    i = 0;
    idle_count = 0;
    do
    {
        if (entry->unk0 == 1 && entry->unk4 == func_8009A390())
        {
            if (cdrom_can_queue_resource((u16)entry->unk4) == 0)
            {
                i++;
                goto next;
            }
            i++;
            entry->unk4 = 0;
            actor = (FieldActorState *)((u8 *)g_field_actor_slots + entry->unk18 * 0x244);
            func_8009A4CC(entry->unkC, actor);
            if (actor->unk25 != 0)
            {
                actor->unk228 = (u8)entry->unkC;
                actor->unk224 |= 1;
                entry->unk0 = 2;
            }
            else
            {
                actor->unk24 = 0;
                func_80084424(D_80105880[0].unkC);
            }
            goto done;
        }
        else
        {
            idle_count++;
            i++;
        }
    next:
        entry++;
    } while (i < 3);
done:
    if (idle_count == 3)
    {
        func_8009A3E8();
    }
}
