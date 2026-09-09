#include "common.h"

/** @brief Partial RecC1B98 layout used by func_800C2724. */
typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 unk4;
    u8 pad5;
    u16 unk6;
    u16 unk8[16];
    s32 unk28;
    s32 unk2C;
    s32 unk30;
    u8 pad34[0x5C];
    s32 unk90;
} RecC1B98;

/** @brief Partial FieldPosition layout used by func_800C2724. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

/** @brief Partial Struct_UnkVec8 layout used by func_800C2724. */
typedef struct
{
    s32 unk0;
    u8 pad4[0x8 - 0x4];
    s32 unk8;
} Struct_UnkVec8;

/** @brief Partial OutRec layout used by func_800C2724. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;
    s32 unk8;
} OutRec;

s32 func_80087770(s32 arg0, s32 arg1);
void func_80087F44(s32 index, FieldPosition *position);
void func_800C1F28(u32 *arg0);
s32 func_800C1FBC(Struct_UnkVec8 *arg0, Struct_UnkVec8 *arg1);

extern u8 *D_80122B78;

/**
 * @brief Collect eligible actors, sort them by distance from actor six, and select one.
 * @return The first sorted actor ID, or 0xFF if none qualified.
 * @note Only records with bits 31 and 29 set and a successful eligibility check qualify.
 * @note WIP: invariant-load placement and saved-register differences remain.
 */
s32 func_800C2724(void)
{
    s32 buf[0x22];
    FieldPosition sp98;
    FieldPosition spA8;
    s32 pad_tail[2];
    s32 i;
    s32 offset;
    s32 flags;
    RecC1B98 *rec;

    func_80087F44(6, &sp98);
    i = 3;
    offset = 0x5EC;
    buf[0] = 0;
    do
    {
        rec = (RecC1B98 *)(D_80122B78 + offset);
        flags = rec->unk90;
        if (flags < 0 && ((((u32)flags) >> 0x1D) & 1) &&
            func_80087770(6, rec->unk0) == 1)
        {
            func_80087F44(rec->unk0, &spA8);
            ((OutRec *)((u8 *)buf + buf[0] * 8))->unk4 = rec->unk0;
            ((OutRec *)((u8 *)buf + buf[0] * 8))->unk8 = func_800C1FBC((Struct_UnkVec8 *)&sp98, (Struct_UnkVec8 *)&spA8);
            buf[0] += 1;
        }
        i += 1;
        offset += 0x94;
    } while (i < 0x10);
    func_800C1F28((u32 *)buf);
    if (buf[0] != 0)
    {
        return ((OutRec *)buf)->unk4;
    }
    return 0xFF;
}
