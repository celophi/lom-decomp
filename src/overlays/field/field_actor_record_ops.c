#include "common.h"


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

void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
RecC1B98 *func_800C1C50(s32 id);
RecC1B98 *func_800C1B60();
void func_800B28E0(s32, s32, s32);

extern u8 *D_80122B78;

/**
 * @brief Update or clear the selected field audio record.
 * @param arg0 Record identifier.
 * @param arg1 Update value, or 0xFF to clear the active flags.
 */
void func_800C2640(s32 arg0, s32 arg1)
{
    RecC1B98 *rec;
    s32 i;

    if (arg1 != 0xFF)
    {
        rec = func_800C1C50(arg0);
        if (rec == NULL)
        {
            akao_set_song_params(0x8001, 1, 1, 1);
            return;
        }
        rec->unk90 |= 0x20000000;
        rec->unk6 = *(u16 *)(D_80122B78 + 0x686);
        i = 0;
        do
        {
            rec->unk8[i] = *(u16 *)(D_80122B78 + 0x688 + i * 2);
            i++;
        } while (i < 16);
        func_800B28E0(arg0, 7, arg1 & 0xFF);
        return;
    }

    rec = func_800C1B60(arg0);
    rec->unk90 &= 0x7FFFFFFF;
    rec->unk90 &= 0xDFFFFFFF;
}

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

extern RecC1B98 *func_800C1B98(void);
extern void func_800C1D14(s32 arg0, s32 arg1);
extern void func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Marks the active record and issues its follow-up actions.
 *
 * When a record is active, sets its 0x40000000 flag, notifies func_800C1D14
 * with the record's leading byte, and - if @p arg1 bit 1 is set - fires
 * func_80087D8C for @p arg0.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain. The former 94.29%
 * result was caused by GCC 2.7.2 CDK epilogue scheduling rather than the C
 * source shape.
 */
void func_800C2848(s32 arg0, s32 arg1)
{
    RecC1B98 *r = func_800C1B98();

    if (r != NULL)
    {
        r->unk90 |= 0x40000000;
        func_800C1D14(r->unk0, arg1);
        if (arg1 & 2)
        {
            func_80087D8C(arg0, -0x400, 0, 0);
        }
    }
}

/**
 * @brief Clear bit 30 of the current record's flag word and re-dispatch its id.
 */
void func_800C28B8(void)
{
    RecC1B98 *p;

    s32 arg0;

    p = func_800C1B60();
    arg0 = p->unk0;
    p->unk90 &= 0xBFFFFFFF;
    func_800C1D14(arg0, 0);
}

