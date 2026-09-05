#include "common.h"

/**
 * @brief Record returned by func_800C1B60: an id byte and a flag word at 0x90.
 */
typedef struct
{
    u8 unk0;
    u8 pad[0x8F];
    s32 unk90;
} UnkStruct800C1B60Ret;

UnkStruct800C1B60Ret *func_800C1B60(void);
void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
u8 *func_800C1E40(s32 arg0);
void *func_800A9060(void);
s32 func_800B2844(s32 arg0, u8 *arg1, s32 arg2);
void func_800A8F8C(void *arg0, u8 *arg1);
void func_800A8FB4(void);
s32 func_800C2AD0(void);

extern u8 D_801148B0[];
extern u8 *D_80122B74;

/**
 * @brief Clear bit 30 of the current record's flag word and re-dispatch its id.
 */
void func_800C28B8(void)
{
    UnkStruct800C1B60Ret *p;

    s32 arg0;

    p = func_800C1B60();
    arg0 = p->unk0;
    p->unk90 &= 0xBFFFFFFF;
    func_800C1D14(arg0, 0);
}

/**
 * @brief Resolve an entry of the first offset table in a 4 KB resource page.
 * @param arg0 Page index into D_801148B0.
 * @param arg1 Entry index within the table.
 * @return Address of the entry.
 */
s32 func_800C28F8(s32 arg0, u16 arg1)
{
    u8 *base = D_801148B0 + (arg0 << 12);
    u8 *table = base + *(s32 *)base;

    return (s32)table + *(s16 *)(arg1 * 2 + table);
}

/**
 * @brief Resolve an entry of the second offset table in a 4 KB resource page.
 * @param arg0 Page index into D_801148B0.
 * @param arg1 Entry index within the table (low 16 bits used).
 * @return Address of the entry.
 */
void *func_800C2928(s32 arg0, s32 arg1)
{
    u8 *base;
    s32 offset;
    s16 *table;

    base = D_801148B0 + (arg0 << 0xC);
    offset = *(s32 *)(base + 4);
    table = (s16 *)(base + offset);
    return (u8 *)table + table[arg1 & 0xFFFF];
}

/**
 * @brief Return the 8-byte record at an index of the third table in a resource page.
 * @param arg0 Page index into D_801148B0.
 * @param arg1 Record index.
 * @return Record address, or NULL when arg1 is out of range.
 */
void *func_800C2958(s32 arg0, u16 arg1)
{
    u8 *rec;
    u8 *tbl;
    u32 count;

    rec = &D_801148B0[arg0 << 12];
    tbl = rec + *(s32 *) (rec + 8);
    count = *(u32 *) tbl;
    if (arg1 < count)
    {
        return tbl + (arg1 * 8 + 4);
    }
    return (void *) 0;
}

/**
 * @brief Set bit arg0 in the bitset at 0x30D4 of the D_80122B74 block.
 * @param arg0 Bit index.
 */
void func_800C299C(s32 arg0)
{
    u8* p = D_80122B74;
    u32 idx = (u32)arg0 >> 5;
    *(s32*)(p + (idx << 2) + 0x30D4) |= 1 << (arg0 & 0x1F);
}

/**
 * @brief Dispatch entry index of resource 5 and attach it to the current object.
 * @param index Entry index; validated against the table's count halfword.
 * @return 0 on success, -1 on any failure.
 * @see decomp.me (100%)
 */
s32 func_800C29CC(s32 index)
{
    u8 *table;
    u8 *entry;
    void *object;

    table = func_800C1E40(5);
    if (table == NULL)
    {
        akao_set_song_params(0x8001, 0x6C, index, 0);
        return -1;
    }
    if (index >= *(u16 *)(table + 2))
    {
        akao_set_song_params(0x8001, 0x6C, index, 1);
        return -1;
    }

    object = func_800A9060();
    entry = table + ((index * 0x40) + 4);
    func_800B2844(0, entry, 0x15);
    if (object != NULL)
    {
        func_800A8F8C(object, entry);
        return 0;
    }
    return -1;
}

/**
 * @brief Deactivate one 0x40-byte record at 0xCE0, or all of them when arg0 >= 0x64.
 * @param arg0 Record index.
 */
void func_800C2A88(s32 arg0)
{
    u8 *p;

    if (arg0 < 0x64)
    {
        p = &D_80122B74[arg0 * 0x40];
        p[0xCE0] = 0;
        func_800A8FB4();
    }
    else
    {
        func_800C2AD0();
    }
}

/**
 * @brief Deactivate all 100 records at 0xCE0 and notify func_800A8FB4.
 * @return Always -1.
 */
s32 func_800C2AD0(void)
{
    s32 i;
    u8 *p;

    for (i = 0; i < 0x64; i++)
    {
        p = &D_80122B74[i * 0x40];
        p[0xCE0] = 0;
    }
    func_800A8FB4();
    return -1;
}
