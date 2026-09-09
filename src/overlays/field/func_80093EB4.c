#include "common.h"
typedef struct { u8 pad0[0x1C]; s32 unk1C; u8 pad20[0xA]; s16 unk2A; u8 pad2C[2]; u16 unk2E; u8 pad30[0xA]; u8 unk3A; } FieldRecord;
typedef struct { u8 pad0[0x4A]; s16 unk4A; u8 pad4C[0x128]; s32 unk174; u8 pad178[1]; u8 unk179; u8 pad17A[0xC2]; } FieldState;
typedef struct { u8 pad0[0x228]; u8 unk228; u8 pad229[0x11]; u8 unk23A; u8 pad23B[9]; } ActorSlot;
void func_8008A678(); void func_800952DC(FieldRecord*,s32); void func_800A2DD8(u8);
extern FieldState D_80105AE0[]; extern ActorSlot g_field_actor_slots[];
void func_80093EB4(FieldRecord *arg0)
{
    ActorSlot *slot;
    FieldState *state;
    FieldState *states = D_80105AE0;
    u8 index;
    u8 slotIndex;
    s32 gate;
    index = arg0->unk3A;
    state = &states[index];
    slotIndex = state->unk179;
    if (slotIndex == 0xFF)
    {
        if (arg0->unk2E != 0) return;
        state->unk174 &= ~0x1800;
        arg0->unk2A = 0;
        arg0->unk1C &= ~0x800;
        states[arg0->unk3A].unk4A = 0;
        if ((u8)arg0->unk3A < 2) func_800A2DD8(arg0->unk3A);
    }
    else
    {
        gate = arg0->unk2E;
        slot = &g_field_actor_slots[slotIndex];
        if (gate != 0 || (slot->unk23A != 0 && slot->unk228 == index)) return;
        func_8008A678();
        arg0->unk2A = 0;
        gate = ~0x1800;
        states[arg0->unk3A].unk174 &= gate;
        if (slot->unk228 == arg0->unk3A) func_800952DC(arg0, 1);
        arg0->unk1C &= ~0x800;
        states[arg0->unk3A].unk4A = 0;
        if ((u8)arg0->unk3A < 2) func_800A2DD8(arg0->unk3A);
    }
}
