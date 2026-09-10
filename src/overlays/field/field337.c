#include "common.h"

typedef struct
{
    u8 pad0[0x21];
    u8 unk21;   /* 0x21 */
    u8 pad22[0x24 - 0x22];
    u8 unk24;   /* 0x24 */
    u8 pad25[0x27 - 0x25];
    u8 unk27;   /* 0x27 */
    u8 pad28[0x2E - 0x28];
    s16 unk2E;  /* 0x2E */
    u8 pad30[0x3A - 0x30];
    u8 unk3A;   /* 0x3A */
} Actor;

typedef struct
{
    u8 pad0[0x174];
    s32 unk174; /* 0x174 */
    u8 pad178[0x23C - 0x178];
} ActorRec;

extern ActorRec D_80105AE0[];


/**
 * @see decomp.me (100%) TODO
 */
void func_80086850(Actor *arg0, s32 arg1)
{
    if (arg1 != 0)
    {
        arg0->unk2E = 1;
        arg0->unk27 = 0;
        arg0->unk21 &= 0x80;
        arg0->unk24 = 1;
        D_80105AE0[arg0->unk3A].unk174 &= ~0x1800;
        func_8006C3FC(arg0);
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0x7);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        func_80086C00(arg0->unk3A);
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_800868FC(Actor *arg0, s32 arg1)
{
    if (arg1 != 0)
    {
        arg0->unk2E = 1;
        arg0->unk27 = 0;
        arg0->unk21 &= 0x80;
        arg0->unk24 = 1;
        D_80105AE0[arg0->unk3A].unk174 &= ~0x1800;
        func_8006C3FC(arg0);
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xA);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
}
