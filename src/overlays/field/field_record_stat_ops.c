#include "common.h"
extern u8 D_800F0BE0[], D_800F0BEC[];
void func_800B7A74(void *, s32, u8 *);

/** @brief Check whether a candidate record is compatible with a slot and its peers. */
s32 func_800B7980(u8 *record, s32 arg1, u8 *arg2)
{
    u8 sp10;
    s32 temp_a2;
    s32 var_v0;
    s32 var_v1;

    temp_a2 = ((u32) (*(u32 *)(arg2 + 0x14)) >> 8) & 3;
    if (temp_a2 != 1)
    {
        if (temp_a2 < 2)
        {
            var_v0 = 0;
            if (temp_a2 != 0)
            {
                return 0;
            }
            if (arg1 == 0)
            {
                func_800B7A74(record, 0, &sp10);
                var_v0 = 0;
                var_v1 = D_800F0BE0[((*(u32 *)(arg2 + 0x14)) >> 10) & 0x3F] & sp10;
                goto block_11;
            }
            /* Duplicate return node #13. Try simplifying control flow for better match */
            return var_v0;
        }
        var_v0 = 0;
        if (temp_a2 != 2)
        {
            return 0;
        }
        var_v1 = arg1 < 4;
        goto block_11;
    }
    var_v0 = 0;
    if ((u32) (arg1 - 1) < 3U)
    {
        func_800B7A74(record, arg1, &sp10);
        var_v0 = 0;
        var_v1 = D_800F0BEC[((*(u32 *)(arg2 + 0x14)) >> 10) & 0x3F] & sp10;
block_11:
        if (var_v1 == 0)
        {
            var_v0 = -1;
        }
    }
    return var_v0;
}



extern u8 D_800F0BE0[];
extern u8 D_800F0BEC[];

void func_800B7A74(void *arg0, s32 arg1, u8 *arg2)
{
    s32 i;
    u8 *p;
    u32 v;
    s32 mode;
    u8 *tbl;

    p = (u8 *)arg0;
    i = 0;
    *arg2 = 0;
    do
    {
        if (i != arg1)
        {
            v = *(u32 *)(p + 0x64);
            mode = (v >> 8) & 3;
            switch (mode)
            {
            case 0:
                tbl = &D_800F0BE0[(v >> 10) & 0x3F];
                break;
            case 1:
                tbl = &D_800F0BEC[(v >> 10) & 0x3F];
                break;
            default:
                p += 0x40;
                i += 1;
                continue;
            }
            v = *arg2;
            v |= *tbl;
            *arg2 = v;
        }
        p += 0x40;
        i += 1;
    } while (i < 4);
}



void func_800B7B98();

s32 func_800B7B08(u8 *arg0, s32 arg1, u8 *arg2)
{
    u8 temp[0x40];
    u8 *slot;

    if (func_800B7980(arg0, arg1, arg2) != 0)
    {
        slot = arg0 + (arg1 * 0x40 + 0x50);
        func_800C1EC8(slot, temp, 0x40);
        func_800C1EC8(arg2, slot, 0x40);
        func_800C1EC8(temp, arg2, 0x40);
        func_800B7B98(arg0);
        return -1;
    }
    return 0;
}

typedef struct {
    u8 pad0[0x18];
    u32 unk18;
    u8 pad1C[0x26 - 0x1C];
    u16 unk26;
    u16 accum[4];
    u16 vals[8];
    u8 pad40[0x74 - 0x40];
    u16 unk74;
} Rec;

void func_800B7B98(Rec *arg0)
{
    s32 i;
    s32 j;
    u8 *entry;
    u16 v;

    arg0->unk26 = arg0->unk74;
    if ((arg0->unk18 & 0x7F) != 3) {
        for (i = 3; i >= 0; i--) {
            arg0->accum[i] = 0;
        }
        for (i = 1; i < 4; i++) {
            entry = (u8 *)arg0 + 0x90 + (i - 1) * 0x40;
            if (*entry != 0) {
                for (j = 0; j < 4; j++) {
                    arg0->accum[j] += *(u16 *)(entry + 0x24 + j * 2);
                }
            }
        }
    }
    for (i = 0; i < 8; i++) {
        v = arg0->vals[i] & 0x1FF;
        arg0->vals[i] = v | ((v >> 2) << 9);
    }
}

/** @brief Per-index field record; only the u16 at +0x24 is used here. */
typedef struct
{
    u8 pad0[0x24];
    u16 unk24;
    u8 pad26[0x22A];
} RecB7C58;

/** @brief Field state block holding the record array at +0x5F0. */
typedef struct
{
    u8 pad0[0x5F0];
    RecB7C58 unk5F0[1];
} StructB7C58;

/** @brief Output record initialized by func_800B7C58. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u32 unk8_low : 24;
    u32 unk8_high : 8;
    s32 unkC;
} OutB7C58;

extern StructB7C58 *D_80122B74;


OutB7C58 *func_80087F0C(s32 index);

/**
 * @brief Initialize an output record from the indexed field record.
 * @param index Field record index.
 */
void func_800B7C58(s32 index)
{
    OutB7C58 *out;

    func_800B7B98((Rec *)&D_80122B74->unk5F0[index]);
    out = func_80087F0C(index);
    out->unk0 = D_80122B74->unk5F0[index].unk24;
    if (out->unk0 == 0)
    {
        out->unk0 = 1;
    }
    out->unk4 = D_80122B74->unk5F0[index].unk24;
    out->unk8_low = D_80122B74->unk5F0[index].unk24;
    out->unkC = 0;
}

/** @brief Eight packed lookup indices for coordinate adjustments. */
typedef struct
{
    u8 pad[0x1C];
    u32 n0 : 4;
    u32 n1 : 4;
    u32 n2 : 4;
    u32 n3 : 4;
    u32 n4 : 4;
    u32 n5 : 4;
    u32 n6 : 4;
    u32 n7 : 4;
} Source;
/** @brief Two seven-bit fields embedded in a packed result word. */
typedef struct
{
    u32 low : 9;
    u32 x : 7;
    u32 middle : 9;
    u32 y : 7;
} Pair;
/** @brief Four packed words adjusted by the source indices. */
typedef struct
{
    u8 pad[0x30];
    Pair pairs[4];
} Result;
extern volatile u8 D_800F0C38[];
/**
 * @brief Apply signed table adjustments to eight packed result fields.
 * @param source Packed four-bit indices into the adjustment table.
 * @param result Destination fields updated modulo 128.
 * @note Volatile table access preserves unsigned loads before sign extension.
 */
void func_800B7D10(Source *source, Result *result)
{
    result->pairs[0].x = (s8)D_800F0C38[source->n0] + result->pairs[0].x;
    {
        u32 y = result->pairs[0].y;
        y += (s8)D_800F0C38[source->n1];
        result->pairs[0].y = y;
    }
    result->pairs[1].x = (s8)D_800F0C38[source->n2] + result->pairs[1].x;
    {
        u32 y = result->pairs[1].y;
        y += (s8)D_800F0C38[source->n3];
        result->pairs[1].y = y;
    }
    result->pairs[2].x = (s8)D_800F0C38[source->n4] + result->pairs[2].x;
    {
        u32 y = result->pairs[2].y;
        y += (s8)D_800F0C38[source->n5];
        result->pairs[2].y = y;
    }
    result->pairs[3].x = (s8)D_800F0C38[source->n6] + result->pairs[3].x;
    {
        u32 y = result->pairs[3].y;
        y += (s8)D_800F0C38[source->n7];
        result->pairs[3].y = y;
    }
}

/** @brief Apply four packed record modifiers and clamp the selected stat to 1-99. */
s32 func_800B7EE8(u8 *arg0, u32 arg1)
{
    u32 var_a2;
    u32 var_v0;
    u32 var_v1;
    u8 *var_a3;

    var_a3 = arg0;
    var_a2 = (u32) (*(u16 *)(arg0 + arg1 * 2 + 0x30) & 0x1FF) >> 2;
    do
    {
        if (var_a3[0x50] != 0)
        {
            switch (arg1)
            {
            case 0:
                var_v0 = (*(u32 *)(var_a3 + 0x6C)) & 0xF;
block_12:
                var_a2 += ((s8 *)D_800F0C38)[var_v0];
                break;
            case 1:
                var_v0 = var_a3[0x6C] >> 4;
                goto block_12;
            case 2:
                var_v0 = ((u32) (*(u32 *)(var_a3 + 0x6C)) >> 8) & 0xF;
                goto block_12;
            case 3:
                var_v0 = ((u32) (*(u32 *)(var_a3 + 0x6C)) >> 0xC) & 0xF;
                goto block_12;
            case 4:
                var_v0 = (*(u16 *)(var_a3 + 0x6E)) & 0xF;
                goto block_12;
            case 5:
                var_v0 = ((u32) (*(u32 *)(var_a3 + 0x6C)) >> 0x14) & 0xF;
                goto block_12;
            case 6:
                var_v0 = var_a3[0x6F] & 0xF;
                goto block_12;
            case 7:
                var_v0 = (u32) (*(u32 *)(var_a3 + 0x6C)) >> 0x1C;
                goto block_12;
            }
        }
        var_a3 += 0x40;
    } while ((s32) var_a3 < (s32) (arg0 + 0x100));
    var_v1 = var_a2;
    if ((s32) var_a2 > 0)
    {
        if ((s32) var_v1 >= 0x64)
        {
            var_v1 = 0x63;
        }
    }
    else
    {
        var_v1 = 1;
    }
    return (s32) var_v1;
}
