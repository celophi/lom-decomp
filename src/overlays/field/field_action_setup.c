#include "common.h"

extern u8 *D_80123FB0;
extern s32 func_800B4CE4(void *, s32);
extern void func_800B2B54(void *, void *, s32, s32, s32, s32);


typedef struct
{
    u8 pad4[4];
    s32 unk4;
} SubState;

typedef struct
{
    s32 unk0;
} EntryB800B5948;

typedef struct
{
    u32 unk0;
    u8 pad4[0x14 - 4];
    s32 unk14;
    SubState *unk18;
    EntryB800B5948 *unk1C;
    s32 unk20;
    s32 unk24;
    u8 pad28[0x4A0 - 0x28];
    u16 unk4A0;
} FieldStateB800B5948;

typedef struct
{
    s32 unk0;
    u8 pad4[0xC - 4];
    s32 unkC;
} ArgB800B5948;



extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);
s32 func_800B2A9C(s32 arg0);
s32 func_800B302C(s32 arg0, s32 arg1);
EntryB800B5948 *func_800B50B8(s32 arg0, void *arg1);

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} ActionCounter5534;

typedef struct
{
    u8 pad0[0xA];
    u16 unkA;
    s32 unkC;
    ActionCounter5534 *unk10;
} ActionActor5534;

typedef struct
{
    s32 unk0;
    u8 pad4[0x14 - 4];
    u32 unk14;
    void *unk18;
    void *unk1C;
    ActionActor5534 *unk20;
    ActionActor5534 *unk24;
} ActionContext5534;

extern void func_8008AB2C(s32, s32);
extern void func_8008B500(s32, s32);
extern void func_800B28E0(s32, s32, s32);
extern s32 func_800B2FF8(void *);
extern void func_800B5948(ArgB800B5948 *, s32);
extern s32 func_800B5A88(void);
extern s32 func_800B5C54(void);
extern void func_800B5D60(s32);
extern void func_800B5E5C(void);
extern s32 func_800B6334(void *);
extern void func_800B65CC(s32);
/* The target forwards the source actor in a0 to the no-argument dispatcher. */
extern s32 func_800B6808();

/** @brief Execute the selected action and process its outcome and actor state. */
s32 func_800B5534(ArgB800B5948 *arg0)
{
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_5;
    s32 var_v0;
    s32 var_v1;
    ActionActor5534 *temp_a0;
    ActionActor5534 *temp_a0_2;
    ActionActor5534 *temp_a0_3;
    ActionActor5534 *temp_a0_4;
    ActionActor5534 *temp_a0_5;
    ActionActor5534 *temp_a0_6;
    ActionActor5534 *temp_a0_7;
    ActionActor5534 *temp_v0_3;
    ActionActor5534 *temp_v0_4;
    ActionActor5534 *var_a1;

    if ((((ActionContext5534 *)D_80123FB0) != NULL) && (((ActionContext5534 *)D_80123FB0)->unk0 >= 0))

    {
        func_800B5948(arg0, -1);
        var_v0 = 1;
        if (((ActionContext5534 *)D_80123FB0)->unk1C != 0)
        {
            temp_v0 = func_800B5C54();
            if (temp_v0 != 0)
            {
                if (temp_v0 == 1)
                {
                    func_800B28E0(arg0->unkC, 0xC, 0);
                }
                var_a1 = ((ActionContext5534 *)D_80123FB0)->unk20;
                var_v1 = var_a1->unkC;
                var_v0 = temp_v0;
                goto block_36;
            }
            temp_v0_2 = func_800B5A88();
            if (temp_v0_2 != 0)
            {
                if (temp_v0_2 == 3)
                {
                    func_8008B500(arg0->unkC, 0x90);
                    goto block_10;
                }
                var_a1 = ((ActionContext5534 *)D_80123FB0)->unk20;
                var_v1 = var_a1->unkC;
                var_v0 = temp_v0_2;
                goto block_36;
            }
            temp_a0 = ((ActionContext5534 *)D_80123FB0)->unk20;
            if (!(temp_a0->unk10->unkC & 0x8000))
            {
                temp_a0->unkC = (s32) (temp_a0->unkC & 0xFFFF00FF);
            }
            if (func_800B6808(temp_a0) != 0)
            {
                func_800B5E5C();
                if (((ActionContext5534 *)D_80123FB0)->unk24->unk10->unk4 == 0)
                {
                    if (!(((u32) ((ActionContext5534 *)D_80123FB0)->unk14 >> 2) & 1))
                    {
                        func_800B28E0(arg0->unk0, 0xC, 5);
                    }
                    func_800B28E0(arg0->unkC, 0xC, 2);
                    if (func_800B4CE4(((ActionContext5534 *)D_80123FB0)->unk20, 3) != 0)
                    {
                        temp_a0_2 = ((ActionContext5534 *)D_80123FB0)->unk24;
                        temp_a0_2->unkC = (s32) (temp_a0_2->unkC | 0x20000000);
                    }
                    if (func_800B4CE4(((ActionContext5534 *)D_80123FB0)->unk20, 0xD) != 0)
                    {
                        temp_a0_3 = ((ActionContext5534 *)D_80123FB0)->unk24;
                        temp_a0_3->unkC = (s32) (temp_a0_3->unkC | 0x02000000);
                    }
                    if (func_800B4CE4(((ActionContext5534 *)D_80123FB0)->unk20, 9) != 0)
                    {
                        temp_a0_4 = ((ActionContext5534 *)D_80123FB0)->unk24;
                        temp_a0_4->unkC = (s32) (temp_a0_4->unkC | 0x0C000000);
                    }
                    if (func_800B2FF8(((ActionContext5534 *)D_80123FB0)->unk20) != 0)
                    {
                        temp_a0_5 = ((ActionContext5534 *)D_80123FB0)->unk24;
                        temp_a0_5->unkC = (s32) (temp_a0_5->unkC | 0x08000000);
                    }
                    if (((ActionContext5534 *)D_80123FB0)->unk20->unkA & 0x100)
                    {
                        temp_v0_3 = ((ActionContext5534 *)D_80123FB0)->unk24;
                        temp_v0_3->unkC = (s32) (temp_v0_3->unkC | 0x10000000);
                    }
                    if (((ActionContext5534 *)D_80123FB0)->unk20->unkA & 0x80)
                    {
                        temp_v0_4 = ((ActionContext5534 *)D_80123FB0)->unk24;
                        temp_v0_4->unkC = (s32) (temp_v0_4->unkC | 0x01000000);
                    }
                    temp_v0_5 = func_800B6334(((ActionContext5534 *)D_80123FB0)->unk24);
                    if (temp_v0_5 != 0)
                    {
                        func_800B65CC(temp_v0_5);
                    }
                }
                else
                {
                    func_800B5D60(1);
                    func_800B28E0(arg0->unk0, 0xC, 4);
                    func_800B28E0(arg0->unkC, 0xC, 1);
                    temp_a0_6 = ((ActionContext5534 *)D_80123FB0)->unk24;
                    temp_a0_6->unkC = (s32) (temp_a0_6->unkC & 0xFFFFFF);
                    func_8008AB2C(arg0->unkC, 0);
                }
                var_a1 = ((ActionContext5534 *)D_80123FB0)->unk20;
                var_v1 = var_a1->unkC;
                var_v0 = 0;
            }
            else
            {
                func_800B5D60(0);
                func_800B28E0(arg0->unkC, 0xC, 0);
                temp_a0_7 = ((ActionContext5534 *)D_80123FB0)->unk24;
                temp_a0_7->unkC = (s32) (temp_a0_7->unkC & 0xFFFFFF);
                var_a1 = ((ActionContext5534 *)D_80123FB0)->unk20;
                var_v0 = 0;
                var_v1 = var_a1->unkC;
            }
block_36:
            var_a1->unkC = (s32) (var_v1 & ~0xFF);

            return var_v0;
        }
        return var_v0;
    }
block_10:
    return 1;
}

/**
 * @brief Update the shared field state for a newly selected record.
 * @param arg0 Record used to initialize the shared state, or NULL to reset it.
 * @param arg1 Whether to resolve and update the associated entry.
 */
void func_800B5948(ArgB800B5948 *arg0, s32 arg1)
{
    EntryB800B5948 *entry;
    s32 val;

    ((FieldStateB800B5948 *)D_80123FB0)->unk18 = (SubState *) arg0;
    if (arg0 == NULL)
    {
        akao_set_song_params(0x8001, (s32) func_800B5948, 0, 0);
        return;
    }
    ((FieldStateB800B5948 *)D_80123FB0)->unk20 = func_800B2A9C(arg0->unk0);
    ((FieldStateB800B5948 *)D_80123FB0)->unk24 = func_800B2A9C(arg0->unkC);
    ((FieldStateB800B5948 *)D_80123FB0)->unk14 = 0;
    ((FieldStateB800B5948 *)D_80123FB0)->unk14 = (((FieldStateB800B5948 *)D_80123FB0)->unk14 & ~2) | ((func_800B302C(arg0->unk0, arg0->unkC) & 1) * 2);
    if (arg1 != 0)
    {
        ((FieldStateB800B5948 *)D_80123FB0)->unk1C = func_800B50B8(-3, ((FieldStateB800B5948 *)D_80123FB0));
        if (func_800B4CE4((void *)((FieldStateB800B5948 *)D_80123FB0)->unk20, 4) != 0)
        {
            entry = ((FieldStateB800B5948 *)D_80123FB0)->unk1C;
            val = entry->unk0;
            if ((u32) (val & 0xF) < 2U)
            {
                entry->unk0 = val | 0xC0;
                ((FieldStateB800B5948 *)D_80123FB0)->unk4A0 = (u16) ((((FieldStateB800B5948 *)D_80123FB0)->unk4A0 * 3) >> 1);
            }
        }
    }
    else
    {
        ((FieldStateB800B5948 *)D_80123FB0)->unk1C = NULL;
    }
}

/** @brief Action record with an owner byte and packed eligibility fields. */
typedef struct
{
    u32 pad0;
    union
    {
        u32 word;
        struct
        {
            u8 id;
            u8 rest[3];
        } bytes;
    } info;
    u16 pad8, flags_a;
} Record;
/** @brief Current action context and its descriptor and actor record pointers. */
typedef struct
{
    u8 pad[0x14];
    u32 flags;
    u32 pad18;
    u32 *descriptor;
    u32 pad20;
    Record *record;
} Context;
s32 func_8008ADB4(u8);
extern u8 *D_80122B74;


/**
 * @brief Classify the current action and mark eligible follow-up actions.
 * @return 0 when unavailable, 1 for the class test, 2 for immediate handling, or 3 for a marked
 * follow-up.
 * @note 100% match with GCC 2.8: 115 instructions, 460 bytes.
 */
s32 func_800B5A88(void)
{
    s32 action;
    u32 mode;
    u32 flags;
    Record *record;
    u8 *source;
    action = func_8008ADB4(((Context *)D_80123FB0)->record->info.bytes.id);
    switch (*((Context *)D_80123FB0)->descriptor & 15)
    {
    case 1:
        if (action == 0x3B)
        {
            return 2;
        }
        if (action == 0x22)
        {
            record = ((Context *)D_80123FB0)->record;
            mode = record->info.word & 0xFC00;
            if (mode == 0 || mode == 0x400)
            {
                source = D_80122B74 + record->info.bytes.id * 0x250;
                if ((u32)(((*(u32 *)(source + 0x654) >> 10) & 63) - 6) < 2)
                {
                    return 1;
                }
            }
        }
        break;
    case 4:
        flags = ((Context *)D_80123FB0)->flags;
        if ((flags >> 1) & 1)
        {
            return 2;
        }
        if ((((Context *)D_80123FB0)->record->flags_a & 0x10) && (u32)(action - 10) < 2)
        {
            ((Context *)D_80123FB0)->flags = flags | 4;
            return 3;
        }
        if (func_800B4CE4(((Context *)D_80123FB0)->record, 0x34))
        {
            ((Context *)D_80123FB0)->flags |= 4;
            return 3;
        }
        break;
    case 5:
        if (func_800B4CE4(((Context *)D_80123FB0)->record, 0x35))
        {
            ((Context *)D_80123FB0)->flags |= 4;
            return 3;
        }
        break;
    case 2:
    case 3:
    case 6:
    default:
        break;
    }
    return 0;
}


typedef struct
{
    u8 pad0[4];
    u8 object_id;
    u8 pad5;
    u16 flags;
} FieldRecordB5C54;

typedef struct
{
    s32 value;
} FieldEntryB5C54;

typedef struct
{
    u8 pad0[0x1C];
    FieldEntryB5C54 *entry;
    void *secondary;
    FieldRecordB5C54 *record;
} FieldStateB5C54;


void field_clear_record_state(FieldRecordB5C54 *record, u32 index);
void func_8008B500(s32 object_id, s32 value);

/**
 * @brief Process the active field record's state flags.
 * @return State result selected by the active record flags.
 */
s32 func_800B5C54(void)
{
    u16 flags;

    flags = ((FieldStateB5C54 *)D_80123FB0)->record->flags;
    if (flags & 0x8000)
    {
        return 1;
    }
    if (flags & 0x4000)
    {
        return 2;
    }
    if (flags & 0x2000)
    {
        if ((((FieldStateB5C54 *)D_80123FB0)->entry->value & 0xF) == 0)
        {
            func_800B2B54(((FieldStateB5C54 *)D_80123FB0)->record, ((FieldStateB5C54 *)D_80123FB0)->secondary, 3, 4, 0x100, 0x12C);
            field_clear_record_state(((FieldStateB5C54 *)D_80123FB0)->record, 6);
            return 2;
        }
    }
    if (((FieldStateB5C54 *)D_80123FB0)->record->flags & 0x1000)
    {
        if ((u32)((((FieldStateB5C54 *)D_80123FB0)->entry->value & 0xF) - 2) < 2)
        {
            func_8008B500(((FieldStateB5C54 *)D_80123FB0)->record->object_id, 0x90);
            return 2;
        }
    }
    return 0;
}


typedef struct
{
    u8 pad0[8];
    s8 unk8;
    u8 unk9;
} Counter;

typedef struct
{
    u8 pad0[8];
    s32 unk8;
} FlagRecord;

typedef struct
{
    u8 pad0[0x18];
    FlagRecord *unk18;
    u8 pad1C[4];
    Counter *unk20;
    Counter *unk24;
} FieldState;



/**
 * @brief Update the field state counters and restart expired counter effects.
 * @param arg0 Amount subtracted from the secondary counter.
 */
void func_800B5D60(s32 arg0)
{
    Counter *primary_counter;
    Counter *secondary_counter;
    Counter *reset_counter;

    primary_counter = ((FieldState *)D_80123FB0)->unk20;
    if (primary_counter->unk8 <= 0)
    {
        primary_counter->unk8 = (s8)primary_counter->unk9;
        func_800B2B54(((FieldState *)D_80123FB0)->unk20, ((FieldState *)D_80123FB0)->unk20, 3, 5, 0x100, 0x3C);
    }
    if (((FieldState *)D_80123FB0)->unk18->unk8 != 0)
    {
        secondary_counter = ((FieldState *)D_80123FB0)->unk24;
        secondary_counter->unk8 = (u8)(secondary_counter->unk8 - arg0);
        reset_counter = ((FieldState *)D_80123FB0)->unk24;
        if (reset_counter->unk8 <= 0)
        {
            reset_counter->unk8 = (s8)reset_counter->unk9;
            func_800B2B54(((FieldState *)D_80123FB0)->unk24, ((FieldState *)D_80123FB0)->unk24, 3, 5, 0x100, 0xB4);
        }
    }
}


typedef struct
{
    u8 pad0[0xC];
    u32 unkC;
    u8 pad10[0x48 - 0x10];
    u16 unk48;
} FieldAccumulator;

typedef struct
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0x10 - 5];
    FieldAccumulator *unk10;
} FieldAccumulatorRecord;

typedef struct
{
    u8 pad0[0x1C];
    u32 *unk1C;
    FieldAccumulatorRecord *unk20;
} FieldAccumulatorState;



/**
 * @brief Advance and clamp a field-state accumulator when its record is eligible.
 */
void func_800B5E5C(void)
{
    FieldAccumulatorRecord *record;
    FieldAccumulator *accumulator;

    record = ((FieldAccumulatorState *)D_80123FB0)->unk20;
    if (((u8)record->unk4 < 2U) && !(record->unk10->unkC & 0x200))
    {
        if (func_800B4CE4(record, 7) != 0)
        {
            FieldAccumulatorState *state;
            s32 index;
            FieldAccumulator *inner;

            state = ((FieldAccumulatorState *)D_80123FB0);
            index = ((u32)*state->unk1C >> 8) & 7;
            inner = state->unk20->unk10;
            inner->unk48 = (u16)(inner->unk48 + (8 << index));
        }
        else
        {
            FieldAccumulatorState *state;
            s32 index;
            FieldAccumulator *inner;

            state = ((FieldAccumulatorState *)D_80123FB0);
            index = ((u32)*state->unk1C >> 8) & 7;
            inner = state->unk20->unk10;
            inner->unk48 = (u16)(inner->unk48 + (4 << index));
        }
        accumulator = ((FieldAccumulatorState *)D_80123FB0)->unk20->unk10;
        if ((u16)accumulator->unk48 >= 0x100U)
        {
            accumulator->unk48 = 0xFFU;
        }
    }
}


typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
} ArgB5F60;

typedef struct
{
    u8 pad0[0x20];
    u8 *unk20;
    u8 *unk24;
} FieldStateB5F60;



s32 func_800B2D34(u8 *arg0, s32 arg1);

s32 rand(void);

/**
 * @brief Roll a random chance against the active field state's scaled threshold.
 * @param arg0 Record forwarded to the shared field-state initializer.
 * @return -1 when the state blocks the action or the roll succeeds; otherwise 0.
 */
s32 func_800B5F60(ArgB5F60 *arg0)
{
    s32 roll;
    s32 remainder;
    s32 chance;
    s32 state;

    state = ((FieldStateB5F60 *)D_80123FB0)->unk24[0x1A];
    if (state >= 100)
    {
        return -1;
    }

    roll = rand() & 0xFFFF;
    remainder = roll % 100;
    func_800B5948((ArgB800B5948 *)arg0, 0);
    chance = func_800B2D34(((FieldStateB5F60 *)D_80123FB0)->unk20, 0);
    if (remainder < (state * chance) / func_800B2D34(((FieldStateB5F60 *)D_80123FB0)->unk24, 4))
    {
        return -1;
    }

    return 0;
}
