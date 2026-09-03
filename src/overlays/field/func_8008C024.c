#include "common.h"

/**
 * @brief Entry record acted on by func_8008C024.
 * @note Only the fields touched by this function are modelled.
 */
typedef struct EntryA0
{
    u8 pad0[0x20];
    u8 unk20;
    u8 pad21[0x2A - 0x21];
    s16 unk2A;
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} EntryA0;

/**
 * @brief Record in the D_800FD818 table indexed by EntryA0::unk3A.
 */
typedef struct RecFD818
{
    u8 pad0[0x260];
    s16 unk260;
    u8 pad262[0x268 - 0x262];
} RecFD818;

/**
 * @brief Slot in the D_80105AE0 table indexed by EntryA0::unk3A.
 */
typedef struct Slot
{
    u8 pad0[0x0C];
    s32 unkC;
    u8 pad10[0x16C - 0x10];
    s8 unk16C;
    u8 pad16D[0x178 - 0x16D];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Slot;

extern RecFD818 D_800FD818[];
extern Slot D_80105AE0[];

/**
 * @brief Reset the field state associated with an entry and re-arm its slot.
 * @param arg0 Entry whose associated table records are updated.
 * @param arg1 Value stored into the slot's unk16C field.
 * @see decomp.me (100%) func_8008C024
 */
void func_8008C024(EntryA0 *arg0, s8 arg1)
{
    if (arg0->unk3A < 3)
    {
        D_800FD818[arg0->unk3A].unk260 = 0;
    }

    D_80105AE0[arg0->unk3A].unkC &= 0x200;
    D_80105AE0[arg0->unk3A].unk178 |= 0x20;
    D_80105AE0[arg0->unk3A].unk16C = arg1;
    arg0->unk2A = 0xAE;
    arg0->unk20 = 0xA;
}
