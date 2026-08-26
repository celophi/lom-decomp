#include "common.h"

typedef struct
{
    s32 unk0;
    u8 pad4[0xC - 4];
    s32 unkC;
    u8 pad10[0x18 - 0x10];
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
extern s32 D_8010CFD4;

void func_800A3B78(s32 arg0);
void func_8009A4A0(s32 arg0);

/**
 * @brief Tear down the actor bound to a track once its slot has gone idle.
 * @param arg0 Track index (clamped to 2 when >= 3 for the D_80105880 lookup).
 * @note Only acts when the track's stored value matches arg0 and the bound
 *       actor slot is free; then it releases the actor, clears the track and
 *       D_8010CFD4, and notifies func_8009A4A0.
 */
void func_80084424(s32 arg0)
{
    s32 value;
    s32 index;
    s32 call_index;
    s32 small;
    Struct_D80105880 *base;
    FieldActorState *actors;

    value = arg0;
    base = D_80105880;
    small = value < 3;
    index = value;
    if (small == 0)
    {
        index = 2;
    }

    if (base[index].unkC == value)
    {
        index = value;
        actors = g_field_actor_slots;
        if (value >= 3)
        {
            index = 2;
        }

        if (actors[base[index].unk18].unk24 == 0)
        {
            func_800A3B78(value);
            index = value;
            if (value >= 3)
            {
                index = 2;
            }

            call_index = value;
            base[index].unk0 = 0;
            D_8010CFD4 = 0;
            if (value >= 3)
            {
                call_index = 2;
            }
            func_8009A4A0(call_index);
        }
    }
}
