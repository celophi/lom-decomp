#include "common.h"

extern u8 *D_80123FB0;
extern u8 *D_80122B78;


typedef struct
{
    s32 unk0;
    s32 unk4;
} StructB2A9C;


StructB2A9C *func_800B2A9C(void);
s32 func_800BD414(s32 arg0, s32 arg1);
void func_800BD520(s32 arg0, u32 arg1, s32 arg2);

/**
 * @see decomp.me (100%)
 */
void func_800B48B8(void)
{
    s32 var_v0;

    if ((D_80123FB0 != NULL) && (*(s32 *)D_80123FB0 >= 0))
    {
        if (func_800B2A9C()->unk4 & 0x200)
        {
            var_v0 = func_800BD414(0, 0x4280);
            func_800BD520(0, 0x4280, var_v0 + 1);
        }
        else
        {
            var_v0 = func_800BD414(0, 0x4284);
            func_800BD520(0, 0x4284, var_v0 + 1);
        }
    }
}


typedef struct ActorB4934
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[5];
    u16 unkA;
} ActorB4934;

extern u16 D_800F0B58[];
extern u8 *D_80122B74;

/**
 * @brief Rebuild an actor's 16-bit status mask from its four sub-entries.
 *
 * Clears the actor's 0xA mask, then walks the four 0x40-byte records that begin
 * at offset 0x5F0 of the actor's per-index block (stride 0x250) inside the table
 * pointed to by @c D_80122B74. For each active record (byte 0 non-zero) it ORs in
 * the 16-bit flag looked up in @c D_800F0B58 by the record's 0x2E field.
 *
 * @param arg0 Actor whose 0x4 index selects the block and whose 0xA mask is set.
 * @note gcc280_g0, 100% match.
 */
void func_800B4934(ActorB4934 *arg0)
{
    s32 i;
    s32 off;
    u8 *base;
    u8 *rec;
    u16 *tbl;

    i = 0;
    tbl = D_800F0B58;
    arg0->unkA = 0;
    off = 0x50;
    base = D_80122B74 + (arg0->unk4 * 0x250 + 0x5F0);
    do
    {
        rec = base + off;
        if (*rec != 0)
        {
            arg0->unkA |= tbl[*(u16 *)(rec + 0x2E)];
        }
        i += 1;
        off += 0x40;
    } while (i < 4);
}


/** @brief Partial FIELD state used by the frame/update dispatcher (see func_800B19FC.c). */
typedef struct
{
    u8 pad0[0xBC];
    s32 unkBC;
    u8 padC0[0x400 - 0xC0];
    s32 unk400;
    u8 pad404[0x418 - 0x404];
    s32 unk418;
} FieldStateB19FC;

typedef struct
{
    u8 pad0[4];
    u32 flags;
    u8 pad8[8];
    u8 *state;
    u8 pad14[0x68 - 0x14];
} FieldStatusRecord;

typedef struct
{
    s32 unk0;
    u8 pad4[0x28 - 4];
    FieldStatusRecord records[11];
} FieldStatusContext;




void func_800B4B44(void);
void func_800B4D1C();
void func_800B4DF0();
void func_800B4E60();
void func_800B4F38();
void func_800B4F80();
s32 func_800BD414(s32 arg0, s32 arg1);

/**
 * @brief Run two update passes over the active field status records.
 */
void func_800B49C0(void)
{
    s32 i;
    s32 keep;

    if ((D_80123FB0 != NULL) && (((FieldStatusContext *)D_80123FB0)->unk0 >= 0))
    {
        i = 0;
        func_800B4B44();
        do
        {
            if ((((FieldStatusContext *)D_80123FB0)->records[i].flags >> 8) & 1)
            {
                if (func_800BD414(0, 0xFFD) == 0)
                {
                    keep = i < 3;
                }
                else
                {
                    if ((u8)((FieldStatusContext *)D_80123FB0)->records[i].flags < 2)
                    {
                        *(u16 *)(((FieldStatusContext *)D_80123FB0)->records[i].state + 0x48) = 0xFF;
                    }
                    keep = i < 3;
                }
                if (keep)
                {
                    func_800B4D1C(&((FieldStatusContext *)D_80123FB0)->records[i]);
                    func_800B4F80(&((FieldStatusContext *)D_80123FB0)->records[i]);
                }
                func_800B4DF0(&((FieldStatusContext *)D_80123FB0)->records[i]);
            }
            i++;
        } while (i < 11);

        i = 0;
        if ((((FieldStateB19FC *)D_80122B78)->unkBC & 0xF) == 0)
        {
            do
            {
                if ((((FieldStatusContext *)D_80123FB0)->records[i].flags >> 8) & 1)
                {
                    func_800B4E60(&((FieldStatusContext *)D_80123FB0)->records[i]);
                    func_800B4F38(&((FieldStatusContext *)D_80123FB0)->records[i]);
                }
                i++;
            } while (i < 11);
        }
    }
}




u8 *func_800A2E34(void);
u32 func_800B4CE4();
void func_800B28E0(s32 arg0, s32 arg1, s32 arg2);
void akao_set_song_params();

/**
 * @brief Rebuild indexed field-state byte mappings and trigger dependent handlers.
 */
void func_800B4B44(void)
{
    s32 i;
    s32 j;
    u8 *list;
    u8 *other;
    s32 first;
    s32 second;
    s32 current;
    s32 entry_index;

    i = 0;
    do
    {
        j = 0;
        do
        {
            u8 *entry;
            entry_index = j + i * 0x68;
            j++;
            entry = D_80123FB0 + entry_index;
            entry[0x75] = 0;
        } while (j < 3);
        i++;
    } while (i < 3);

    i = 0;
    list = func_800A2E34();
    while (*list != 0xFF)
    {
        other = list + 1;
        current = *list;
        {
            s32 dst_offset;
            s32 src_offset;
            u8 *base;
            u8 *src;
            u8 *dst;
            u8 value;

            first = *other;
            dst_offset = i + current * 0x68;
            src_offset = first * 0x68;
            base = D_80123FB0;
            src = base + src_offset;
            value = src[0x74];
            dst = base + dst_offset;
            dst[0x75] = value;
        }
        second = *other;
        other += 2;
        first = *list;
        list += 2;
        {
            s32 dst_offset;
            s32 src_offset;
            u8 *base;
            u8 *src;
            u8 *dst;
            u8 value;

            dst_offset = i + second * 0x68;
            src_offset = first * 0x68;
            base = D_80123FB0;
            src = base + src_offset;
            value = src[0x74];
            dst = base + dst_offset;
            dst[0x75] = value;
        }
        i++;
    }

    if (func_800B4CE4(D_80123FB0 + 0x90, 0) < 3)
    {
        func_800B28E0(1, 0xC, 6);
    }
    if (func_800B4CE4(D_80123FB0 + 0xF8, 0) < 3)
    {
        func_800B28E0(2, 0xC, 6);
    }
    if (i >= 4)
    {
        akao_set_song_params(0x8001, 0x6F, i, 0);
    }
}


extern void field_clear_record_state(void *, u32);


typedef struct UnkStruct800B4CE4
{
    u8 pad0[0x4D];
    u8 values[3];
} UnkStruct800B4CE4;

u32 func_800B4CE4(UnkStruct800B4CE4 *arg0, s32 arg1)
{
    u32 count;
    u32 i;

    i = 0;
    count = i;
    for (; i < 3; i++)
    {
        if (arg0->values[i] == arg1)
        {
            count++;
        }
    }
    return count;
}


typedef struct StateB4D1C
{
    u8 pad0[0x48];
    u16 unk48;
} StateB4D1C;

typedef struct RecordB4D1C
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0xB];
    StateB4D1C *state;
} RecordB4D1C;

void func_800B2D64(RecordB4D1C *arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Clear selected record state and apply active field record actions.
 * @param arg0 Record whose active entries are processed.
 */
void func_800B4D1C(RecordB4D1C *arg0)
{
    s32 i;
    s32 j;

    if (func_800B4CE4((UnkStruct800B4CE4 *)arg0, 5) != 0)
    {
        field_clear_record_state(arg0, 0xFF);
    }

    i = 0x60;
    do
    {
        if (func_800B4CE4((UnkStruct800B4CE4 *)arg0, i) != 0)
        {
            field_clear_record_state(arg0, i - 0x60);
        }
        i++;
    } while (i < 0x6C);

    if (func_800B4CE4((UnkStruct800B4CE4 *)arg0, 6) != 0)
    {
        if (arg0->unk4 == 0)
        {
            arg0->state->unk48 = 0xFF;
        }
    }

    j = 0x70;
    do
    {
        if (func_800B4CE4((UnkStruct800B4CE4 *)arg0, j) != 0)
        {
            func_800B2D64(arg0, j - 0x70, 0xA, 0);
        }
        j++;
    } while (j < 0x80);
}


typedef struct StateB3160
{
    u8 pad0[0xC];
    u32 flags;
} StateB3160;

typedef struct RecordB3160
{
    u8 pad0[0x10];
    StateB3160 *state;
} RecordB3160;


/**
 * @brief Ticks a record's twelve state timers and clears any that expire.
 *
 * For each of the twelve half-word timers at record offset 0x50, decrements a
 * nonzero timer and, when it reaches zero or below, clears that state via
 * field_clear_record_state.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain. The former 94.29%
 * result was caused by routing this standalone unit through GCC 2.7.2 CDK,
 * whose epilogue scheduling differs from the target.
 */
void func_800B4DF0(RecordB3160 *record)
{
    s32 i;

    for (i = 0; i < 12; i++)
    {
        if (*(s16 *)((u8 *)record + i * 2 + 0x50) != 0)
        {
            s16 nv = *(u16 *)((u8 *)record + i * 2 + 0x50) - 1;
            *(s16 *)((u8 *)record + i * 2 + 0x50) = nv;
            if (nv <= 0)
            {
                field_clear_record_state(record, i);
            }
        }
    }
}


typedef struct Inner {
    u32 unk0;
    s32 unk4;
    u8 pad8[4];
    u32 unkC;
} Inner;

typedef struct Other {
    u8 pad0[0x3F];
    u8 unk3F;
} Other;

typedef struct Rec {
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0xB];
    Inner *unk10;
    Other *unk14;
} Rec;

/**
 * @see decomp.me (100%)
 */
void func_800B4E60(Rec *arg0)
{
    s32 var_a2;
    u32 var_v0;
    s32 var_v1;
    Inner *var_a0;
    Inner *var_a1;

    var_v0 = arg0->unk4;
    var_a1 = arg0->unk10;
    var_v0 = var_v0 < 3U;
    var_a2 = var_a1->unk4;
    if (var_v0 != 0) {
        var_v0 = var_a1->unkC;
        var_v0 &= 0x190;
        if (var_v0 != 0) {
            var_v0 = var_a1->unk0;
            var_v0 >>= 5;
            do {
                var_v1 = 1;
                if (var_v0 != 0) {
                    var_v1 = var_v0;
                }
                var_a2 -= var_v1;
            } while (0);
            var_v0 = 1;
            if (var_a2 > 0) {
                var_a1->unk4 = var_a2;
                return;
            }
            var_a1->unk4 = var_v0;
        }
    } else {
        var_v0 = var_a1->unkC;
        var_v0 &= 0x191;
        if (var_v0 != 0) {
            var_v0 = arg0->unk14->unk3F;
            var_v0 &= 0x80;
            do {
                var_v1 = 1;
                if (var_v0 != 0) {
                    var_v0 = var_a1->unk0;
                    var_v0 >>= 8;
                } else {
                    var_v0 = var_a1->unk0;
                    var_v0 >>= 5;
                }
                if (var_v0 != 0) {
                    var_v1 = var_v0;
                }
                var_a2 -= var_v1;
            } while (0);
            var_a0 = arg0->unk10;
            var_v0 = 1;
            if (var_a2 > 0) {
                var_a0->unk4 = var_a2;
                return;
            }
            var_a0->unk4 = var_v0;
        }
    }
}


void func_800B4F38(u8 *arg0)
{
    s32 i;
    u8 *p;
    u8 current;
    u8 target;

    for (i = 0; i < 8; i++)
    {
        p = &arg0[i];
        current = p[0x28];
        target = p[0x30];
        if (target < current)
        {
            p[0x28] = current - 1;
        }
        else if (current < target)
        {
            p[0x28] = current + 1;
        }
    }
}


s32 func_8008ADB4(u8 arg0);
s32 func_800B2D34(u8 *arg0, s32 arg1);
void saturating_counter_add(void *counter, s32 delta);

extern u8 *D_80122B78;

/**
 * @brief Update the record's saturating counters when its growth interval elapses.
 * @param arg0 Record containing the growth state and linked counter.
 */
void func_800B4F80(u8 *arg0)
{
    u16 flags;
    s32 multiplier;
    s32 scaled_remaining;
    s32 divisor;
    s32 classification;

    flags = *(u16 *)(arg0 + 0xA);
    if (flags & 1)
    {
        return;
    }
    if (*(s32 *)(*(u8 **)(arg0 + 0x10) + 0xC) & 0x391)
    {
        return;
    }
    if ((flags & 8) || func_800B4CE4((UnkStruct800B4CE4 *)arg0, 2) != 0)
    {
        multiplier = 8;
    }
    else
    {
        classification = func_8008ADB4(arg0[4]);
        if (classification < 0)
        {
            return;
        }
        if (classification < 2)
        {
            multiplier = 4;
        }
        else if (classification != 0x31)
        {
            return;
        }
        else
        {
            multiplier = 2;
        }
    }

    scaled_remaining = (0x64 - func_800B2D34(arg0, 4)) * multiplier;
    if (scaled_remaining < 0)
    {
        scaled_remaining += 0xF;
    }
    divisor = scaled_remaining >> 4;
    if (divisor <= 0)
    {
        divisor = 1;
    }
    if ((u32)(*(s32 *)(D_80122B78 + 0xBC)) % (u32)divisor == 0)
    {
        saturating_counter_add(*(u8 **)(arg0 + 0x10), 1);
        if (arg0[4] < 3)
        {
            saturating_counter_add(*(u8 **)(arg0 + 0x10), func_800B4CE4((UnkStruct800B4CE4 *)arg0, 1));
        }
    }
}
