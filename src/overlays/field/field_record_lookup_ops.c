#include "common.h"

/**
 * @brief 0x94-byte actor record in the table at D_80122B78 + 0x430.
 */
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

/** @brief Block at D_80122B78: record count halfword at 0x400, 16 records at 0x430. */
typedef struct
{
    u8 pad0[0x400];
    u16 unk400;
    u8 pad402[0x2E];
    RecC1B98 unk430[16];
} StructC1B98;

typedef struct
{
    s32 unk0;
} SomeRec;

#define REC_TABLE ((StructC1B98 *)D_80122B78)

SomeRec *func_80087F0C(s32 arg0);
void saturating_counter_add(SomeRec *counter, s32 delta);
void func_8008BD88(s32 arg0);
u32 *func_800875B4(void);
void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);

/*
 * Declared without a prototype: func_800C1B60 forwards its caller's a0 to
 * func_800C1B98 by calling it with no arguments, which a prototype would
 * reject. The definition below carries the real signature.
 */
RecC1B98 *func_800C1B98();
RecC1B98 *func_800C1B60();

extern u8 *D_80122B78;


/** @brief Partial RetC1B60 layout used by func_800C1A18. */
typedef struct
{
    u8 unk0[2];
    u16 unk2;
} RetC1B60;

/** @brief Partial SlotRec layout used by func_800C1A18. */
typedef struct
{
    u8 pad0[2];
    u16 unk2;
    s32 unk4;
    u8 pad8[0x44 - 8];
} SlotRec;

/** @brief Partial FieldStateB layout used by func_800C1A18. */
typedef struct
{
    u8 pad0[8];
    SlotRec *unk8;
} FieldStateB;

void func_800C2138(s32 arg0);
u8 *func_800A9060(void);
void func_800A8F8C(u8 *dst, u8 *src);
void func_800A8D8C(u8 *arg0, u8 arg1);

extern FieldStateB *D_80123FB0;
extern u16 D_800F0E98[];

/**
 * @brief Resolve a command to a slot record or table entry and submit its data.
 * @param arg0 Unused.
 * @param arg1 Command source passed to func_800C1B60.
 * @note WIP: two target instructions and temporary-register differences remain.
 */
void func_800C1A18(void *arg0, void *arg1)
{
    RetC1B60 *ret;
    u16 flags;
    s32 code;
    s32 i;
    s32 count;
    SlotRec *table;
    SlotRec *cursor;
    s32 offset;
    u8 *found;
    u8 *handle;
    u8 *arg0_2;
    u8 arg1_2;

    ret = (RetC1B60 *)func_800C1B60(arg1);
    flags = ret->unk2;
    code = flags & 0xFF;
    if (!(flags & 0x8000))
    {
        s32 key;

        found = NULL;
        table = D_80123FB0->unk8;
        offset = 0;
        count = table->unk2;
        i = offset;
        if (count != 0)
        {
            s32 n;
            SlotRec *base;

            key = flags & 0xFFFF;
            base = table;
            n = count;
            cursor = base;
            i = 0;
            do
            {
                if (cursor->unk4 == key)
                {
                    found = (u8 *)base + offset + 8;
                    goto found_match;
                }
                cursor = (SlotRec *)((u8 *)cursor + 0x44);
                offset += 0x44;
                i += 1;
            } while (i < n);
found_match:;
        }
        if (found == NULL)
        {
            return;
        }
        handle = func_800A9060();
        if (handle == NULL)
        {
            return;
        }
        func_800A8F8C(handle, found);
        arg0_2 = handle;
        arg1_2 = 0;
        func_800A8D8C(arg0_2, arg1_2);
        return;
    }
    else
    {
        func_800C2138(code);
        arg0_2 = (u8 *)D_800F0E98 + D_800F0E98[code];
        arg1_2 = 1;
    }
    func_800A8D8C(arg0_2, arg1_2);
}

/**
 * @brief Scale a counter's value by arg1 / 256 and apply it through saturating_counter_add.
 * @param arg0 Counter id passed to func_80087F0C.
 * @param arg1 Multiplier in 1/256 units.
 */
void func_800C1B20(s32 arg0, s32 arg1)
{
    SomeRec *rec;
    s32 product;

    rec = func_80087F0C(arg0);
    product = rec->unk0 * arg1;
    saturating_counter_add(rec, (u32) product >> 8);
}

/**
 * @brief Look up an actor record, falling back to the default record at 0xE04.
 *
 * Takes no formal parameters so that the caller's a0 flows unchanged into
 * func_800C1B98; callers pass the record id in that slot.
 *
 * @return The matching record, or the default record when none matched.
 */
RecC1B98 *func_800C1B60()
{
    RecC1B98 *var_v0;

    var_v0 = func_800C1B98();
    if (var_v0 == 0)
    {
        var_v0 = (RecC1B98 *)(D_80122B78 + 0xE04);
    }
    return var_v0;
}

/**
 * @brief Find the record for an actor id.
 * @param id Ids below 3 index directly, 3..0x7F search active records, 0x80+ map to slot id - 0x70.
 * @return The record, or NULL when a searched id is not present.
 * @see decomp.me (100%)
 */
RecC1B98 *func_800C1B98(s32 id)
{
    s32 i;

    if (id < 3)
    {
        return &REC_TABLE->unk430[id];
    }
    if (id < 0x80)
    {
        for (i = 0; i < 16; i++)
        {
            if ((REC_TABLE->unk430[i].unk90 < 0) && (REC_TABLE->unk430[i].unk0 == id))
            {
                goto found;
            }
        }
        return NULL;
    }
    return &REC_TABLE->unk430[id - 0x70];
found:
    return &REC_TABLE->unk430[i];
}

/**
 * @brief Claim the first free record slot for an actor id and reset it.
 * @param id Actor id to store in the slot.
 * @return The claimed record, or NULL when all 16 slots are active.
 * @see decomp.me (100%)
 */
RecC1B98 *func_800C1C50(s32 id)
{
    s32 i;
    s32 j;

    for (i = 0; i < 16; i++)
    {
        if (REC_TABLE->unk430[i].unk90 >= 0)
        {
            REC_TABLE->unk430[i].unk0 = id;
            REC_TABLE->unk430[i].unk4 = 0xFF;
            REC_TABLE->unk430[i].unk6 = 0xFFFF;
            REC_TABLE->unk430[i].unk90 &= ~0xF;
            REC_TABLE->unk430[i].unk90 &= ~0x20000000;
            REC_TABLE->unk430[i].unk90 &= ~0x40000000;
            REC_TABLE->unk430[i].unk90 |= 0x80000000;
            for (j = 0; j < 16; j++)
            {
                REC_TABLE->unk430[i].unk8[j] = 0xFFFF;
            }
            return &REC_TABLE->unk430[i];
        }
    }
    return NULL;
}

/**
 * @brief Release an actor's record, optionally notifying func_8008BD88 first.
 * @param arg0 Actor id.
 * @param arg1 Bit 0 set requests the func_8008BD88 notification.
 */
void func_800C1D14(s32 arg0, s32 arg1)
{
    s32 temp_a1;
    RecC1B98 *temp_v0;

    temp_a1 = arg1 & 1;
    if (temp_a1 != 0)
    {
        func_8008BD88(arg0);
    }
    temp_v0 = func_800C1B60(arg0);
    temp_v0->unk2C = 0;
    temp_v0->unk30 = 0;
    temp_v0->unk28 = temp_v0->unk28 & 0x7FFFFFFF;
}

/**
 * @brief Release every record whose flag bit 30 is clear.
 */
void func_800C1D68(void)
{
    s32 i;
    s32 off;
    u8 *rec;

    for (i = 0; i < *(u16 *)(D_80122B78 + 0x400); i++)
    {
        off = i * 0x94;
        rec = D_80122B78 + off;
        if (!((*(u32 *)(rec + 0x4C0) >> 30) & 1))
        {
            *(u16 *)(rec + 0x436) &= 0xFEFF;
            func_800C1D14(rec[0x430], 1);
        }
    }
}

/**
 * @brief Empty loop over the record count; the body was compiled away.
 */
void func_800C1E08(void)
{
    s32 var_v1;
    u16 temp_v0;

    var_v1 = 0;
    temp_v0 = *(u16 *)(D_80122B78 + 0x400);
    if (temp_v0 != 0)
    {
        do
        {
            var_v1 += 1;
        } while (var_v1 < (s32) temp_v0);
    }
}

/**
 * @brief Find the resource record whose leading halfword equals arg0.
 * @param arg0 Resource id to look for.
 * @return The record, or NULL after reporting the miss to the audio driver.
 */
u16 *func_800C1E40(s32 arg0)
{
    u32 *base;
    u16 *record;
    s32 i;
    u32 count;

    base = func_800875B4();
    count = (u32) base[0] >> 2;
    for (i = 0; i < (s32) count; i += 1)
    {
        record = (u16 *) ((u8 *) base + base[i]);
        if (*record == arg0)
        {
            return record;
        }
    }
    akao_set_song_params(0x8001, 0x6B, arg0, 0);
    return NULL;
}
