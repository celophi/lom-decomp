#include "common.h"


/** @brief 0x94-byte actor record stored in the FIELD actor table. */
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

/** @brief FIELD actor table containing the 16 records scanned by func_800C2724. */
typedef struct
{
    u8 pad0[0x400];
    u16 unk400;
    u8 pad402[0x2E];
    RecC1B98 records[16];
} FieldActorTable;

/** @brief Three-dimensional field position. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

/** @brief Actor identifier paired with its distance from a reference position. */
typedef struct
{
    s32 object_id;
    s32 distance;
} FieldDistanceEntry;

/** @brief Sortable list of actor identifiers and their distances. */
typedef struct
{
    s32 count;
    FieldDistanceEntry entries[16];
} FieldDistanceList;

void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
RecC1B98 *func_800C1C50(s32 id);
RecC1B98 *func_800C1B60();
void func_800B28E0(s32, s32, s32);
s32 func_80087770(s32 arg0, s32 arg1);
void func_80087F44(s32 index, FieldPosition *position);
void func_800C1F28(u32 *arg0);
s32 func_800C1FBC(FieldPosition *arg0, FieldPosition *arg1);

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

/**
 * @brief Select the nearest eligible actor to actor six.
 * @return Object ID of the nearest eligible actor, or 0xFF when none qualify.
 */
s32 func_800C2724(void)
{
    FieldDistanceList list;
    FieldPosition reference_position;
    FieldPosition actor_position;
    s32 actor_index;
    s32 flags;
    RecC1B98 *actor;

    func_80087F44(6, &reference_position);
    actor_index = 3;
    list.count = 0;
    do
    {
        actor = &((FieldActorTable *)D_80122B78)->records[actor_index];
        flags = actor->unk90;
        if (flags < 0 && ((((u32)flags) >> 0x1D) & 1) &&
            func_80087770(6, actor->unk0) == 1)
        {
            func_80087F44(actor->unk0, &actor_position);
            list.entries[list.count].object_id = actor->unk0;
            list.entries[list.count].distance = func_800C1FBC(&reference_position, &actor_position);
            list.count += 1;
        }
        actor_index += 1;
    } while (actor_index < 16);

    func_800C1F28((u32 *)&list);
    if (list.count != 0)
    {
        return list.entries[0].object_id;
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

