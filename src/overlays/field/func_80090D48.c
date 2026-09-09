#include "common.h"

/** @brief Animation entry with state, frame counters, variant, and actor slot. */
typedef struct
{
    u8 pad0[0x1C];
    s32 unk1C;
    u8 pad20;
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 pad25[2];
    u8 unk27;
    u8 pad28[2];
    u16 unk2A;
    u8 pad2C[2];
    u16 unk2E;
    union
    {
        u16 half;
        u8 byte[2];
    } variant;
    u8 pad32[8];
    u8 unk3A;
} Entry;
/** @brief Actor fields affected by animation commands. */
typedef struct
{
    u8 pad0[12];
    s32 unkC;
    u8 pad10[4];
    s32 unk14;
    u8 pad18[0x3C - 0x18];
    s32 unk3C;
    u8 pad40[8];
    u16 unk48;
    u8 pad4A[0x174 - 0x4A];
    s32 unk174;
} Actor;
/** @brief Animation command code and argument. */
typedef struct
{
    union
    {
        u16 half;
        u8 byte[2];
    } code;
    u8 pad2[4];
    u16 unk6;
} Command;
/** @brief Actor slot record containing the type byte. */
typedef struct
{
    u8 pad0;
    u8 unk1;
    u8 pad2[0x268 - 2];
} ActorData;
extern ActorData D_800FD818[];
extern u8 D_800EB1B8[];
extern void func_8006C3FC(Entry *);
extern void func_800A3938(s32, s32);
extern void func_800B61C4(s32);
/**
 * @brief Apply an animation command and its actor and sound side effects.
 * @param arg0 Animation entry to update.
 * @param arg1 Actor receiving the command.
 * @param arg2 Animation command and argument.
 */
void func_80090D48(Entry *arg0, Actor *arg1, Command *arg2)
{
    s32 temp_v0;
    s32 temp_v1_2;
    s32 temp_v1;
    s32 temp_v1_3;
    s32 var_v0;
    s32 temp_v1_4;
    u8 *sound_base;
    u8 *sound;

    arg1->unk3C = (s32)arg2->unk6;
    temp_v1 = arg2->code.half;
    if (((u32)(temp_v1 - 0x2F) < 2U) || (temp_v1_2 = temp_v1 & 0xFFFF, (temp_v1_2 == 0x44)) ||
        (temp_v1_2 == 0x45))
    {
        arg1->unkC = (s32)(arg1->unkC | 0x4000);
    }
    if ((arg2->code.half == 0x1F) && (arg0->variant.half != 0) &&
        ((*(volatile u8 *)&arg0->unk3A >= 2U) || (D_800FD818[arg0->unk3A].unk1 != 0xA)))
    {
        arg0->unk21 = (u8)(arg0->variant.byte[0] + (arg2->code.byte[0] + (arg0->unk21 & 0x80)));
    }
    else
    {
        arg0->unk21 = (u8)(arg2->code.byte[0] + (arg0->unk21 & 0x80));
    }
    arg0->unk2E = 1;
    arg0->unk27 = 0;
    arg0->unk24 = 1;
    arg1->unk174 = (s32)(arg1->unk174 & ~0x1800);
    func_8006C3FC(arg0);
    temp_v0 = arg0->unk21 & 0x7F;
    switch (temp_v0)
    {
    case 0x41:
        func_800B61C4(arg1->unk14);
        goto set_state;
    case 0x33:
        temp_v1_3 = arg1->unk48;
        var_v0 = temp_v1_3 < 0xE0U ? temp_v1_3 + 0x20 : 0xFF;
        arg1->unk48 = var_v0;
        goto set_state;
    case 0x8:
    case 0xA:
    case 0x31:
    case 0x3D:
        arg0->unk2A = 0x86;
        arg0->unk1C = (s32)(arg0->unk1C | 0x800);
        goto play_sound;
    default:
        goto set_state;
    }
    goto play_sound;
set_state:
    arg0->unk2A = 0x86;
play_sound:
    if ((u8)arg0->unk3A < 2U)
    {
        sound_base = D_800EB1B8;
        temp_v1_4 = (s32)&sound_base[arg0->unk21 & 0x7F];
        temp_v1_4 = *(u8 *)temp_v1_4;
        if (temp_v1_4 != 0xFF)
        {
            func_800A3938(temp_v1_4, 0x80);
        }
    }
}
