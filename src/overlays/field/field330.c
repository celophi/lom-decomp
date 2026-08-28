#include "common.h"

typedef struct { u8 pad0[0x3A]; u8 unk3A; } Actor;
typedef struct {
    u8 pad0[0x24];
    u8 unk24;   /* 0x24 */
    u8 pad25[0x23A - 0x25];
    u8 unk23A;  /* 0x23A */
    u8 pad23B[0x244 - 0x23B];
} ActorSlot;

extern ActorSlot g_field_actor_slots[];

/**
 * @see decomp.me (100%) TODO
 */
void func_80086DD0(Actor *arg0, s32 arg1)
{
    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xE);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
    else
    {
        g_field_actor_slots[arg0->unk3A + 0x40].unk24 = 0;
        g_field_actor_slots[arg0->unk3A + 0x40].unk23A = 0;
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80086E78(Actor *arg0, s32 arg1)
{
    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0x19);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
    else
    {
        g_field_actor_slots[arg0->unk3A + 0x40].unk24 = 0;
        g_field_actor_slots[arg0->unk3A + 0x40].unk23A = 0;
    }
}
