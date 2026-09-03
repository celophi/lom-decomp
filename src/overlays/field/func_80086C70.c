#include "common.h"
typedef struct { u8 pad0[0xC]; s32 unkC; u8 pad10[0x170 - 0x10]; u8 unk170; u8 pad171[0x178 - 0x171]; u32 unk178; u8 pad17C[0x23C - 0x17C]; } ActorSlotData;
typedef struct { u8 pad0[0x25]; u8 unk25; u8 pad26[0x3A - 0x26]; u8 unk3A; } Actor;
extern ActorSlotData D_80105AE0[];
void func_80083EEC(s32 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation(s32 arg0, s32 arg1, s32 arg2);

void func_80086C70(Actor *arg0, s32 arg1)
{
    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xC);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        arg0->unk25 = 0xFE;
        {
            ActorSlotData *base = D_80105AE0;
            ActorSlotData *e = &base[arg0->unk3A];
            if ((e->unk178 >> 1) & 1)
            {
                ActorSlotData *e2 = &base[e->unk170];
                e2->unkC &= ~0x2000;
            }
        }
    }
    else
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xD);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        arg0->unk25 = 0;
    }
}
