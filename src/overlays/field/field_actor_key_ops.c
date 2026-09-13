#include "common.h"

/*
 * Consolidated FIELD actor-key operations TU (vram 0x80087614 .. 0x800880EC).
 *
 * Each of these functions looks up a field actor by key/selector (most via
 * func_80087C9C) and reads or mutates the parallel D_80105AE0 / D_800FDF58
 * actor tables. Every member viewed those shared arrays through its own partial
 * struct layout, so D_80105AE0 and D_800FDF58 are declared at BLOCK scope inside
 * each function with that function's original record type; there is deliberately
 * no file-scope declaration of either symbol.
 */

/* ------------------------------------------------------------------ *
 * File-scope record types (kept distinct per originating function).  *
 * ------------------------------------------------------------------ */

/** @brief D_80105AE0 slot as seen by func_80087C9C / func_80087F44. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} RecA80105AE0;

/** @brief D_800FDF58 record with three leading words (func_80087F44). */
typedef struct
{
    s32 unk0; /* 0x0 */
    s32 unk4; /* 0x4 */
    s32 unk8; /* 0x8 */
    u8 padC[0x54 - 0xC];
} RecB800FDF58_F307;

/** @brief Output triple written by func_80087F44. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} OutRec;

/** @brief D_800FDF58 record as an opaque 0x54-byte blob (func_80087C9C). */
typedef struct
{
    u8 data[0x54];
} RecB800FDF58_F32;

/** @brief D_80105AE0 slot as seen by func_80087F0C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0;

/** @brief Record carrying the state-table index (func_80087614). */
typedef struct Record87614
{
    u8 pad0[0x3A];
    u8 index;
} Record87614;

/** @brief D_80105AE0 slot with low flags at 0x10 (func_80087614). */
typedef struct State87614
{
    u8 pad0[0x10];
    s32 flags;
    u8 pad14[0x23C - 0x14];
} State87614;

/** @brief Actor record updated by func_80087680. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x1C - 0xC];
    union
    {
        s32 w;
        struct
        {
            u16 lo;
            u16 hi;
        } h;
    } unk1C;
    u8 pad20[0x25 - 0x20];
    u8 unk25;
    u8 pad26[0x3A - 0x26];
    u8 unk3A;
} Rec80087680;

/** @brief D_80105AE0 slot with low flags at 0x10 (func_80087680). */
typedef struct
{
    u8 pad0[0x10];
    s32 flags;
    u8 pad14[0x23C - 0x14];
} State80087680;

/** @brief Actor position and packed facing direction (func_80087770). */
typedef struct
{
    s32 position_x;
    u8 pad4[4];
    s32 position_z;
    u8 pad12[0x21 - 0xC];
    u8 facing_flags;
} FacingActorRecord;

/** @brief Record carrying the binding selector (func_800878B4). */
typedef struct
{
    u8 pad[0x3A];
    u8 selector;
} Record;

/** @brief Binding state, owner index, and actor slot (func_800878B4). */
typedef struct
{
    s32 state;
    u8 pad4[8];
    s32 owner;
    u8 pad10[8];
    s32 slot;
} Binding;

/** @brief Actor slot with two state bytes tested by func_800878B4. */
typedef struct
{
    u8 pad[0x23A];
    u8 first, second;
    u8 tail[8];
} Actor878B4;

/** @brief Actor entry with position, preserved state bits, slot (func_80087A9C). */
typedef struct
{
    s32 unk0, unk4, unk8;
    u8 padC[16];
    union
    {
        s32 word;
        u16 half[2];
    } state;
    u8 pad20;
    s8 unk21;
    u8 pad22[3];
    u8 unk25;
    u8 pad26[20];
    u8 unk3A;
} Entry;

/** @brief Actor slot owner and group flags in a 0x23C-byte record (func_80087A9C). */
typedef struct
{
    u8 pad0[16];
    s32 unk10, unk14;
    u8 pad18[0x23C - 0x18];
} ActorA9C;

/** @brief Resource flags in a 0x14-byte entry (func_80087A9C). */
typedef struct
{
    u8 pad0[16];
    s32 unk10;
} Resource;

/** @brief Per-actor animation/geometry slot, stride 0x23C (func_80087CE0). */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} FieldActorSlotCE0;

/** @brief Parallel per-actor record, stride 0x54 (func_80087CE0). */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x28 - 0x0C];
    u8 unk28;
    u8 pad29;
    s16 unk2A;
    s16 unk2C;
    u8 pad2E[0x54 - 0x2E];
} FieldActorRecordCE0;

/** @brief Per-actor slot with extra word at 0x168, stride 0x23C (func_80087E00). */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x168 - 0x18];
    s32 unk168;
    u8 pad16C[0x23C - 0x16C];
} FieldActorSlotE00;

/** @brief Parallel per-actor record with slot index at 0x3A (func_80087E00). */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x28 - 0x0C];
    u8 unk28;
    u8 pad29;
    s16 unk2A;
    s16 unk2C;
    u8 pad2E[0x3A - 0x2E];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldActorRecordE00;

/** @brief Partial D_80105AE0 slot layout used by func_80087FC0. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} StateB80087FC0;

/** @brief Partial D_800FDF58 record layout used by func_80087FC0. */
typedef struct
{
    u8 pad0[0x10];
    u16 unk10;
    u8 pad12[0x1C - 0x12];
    u32 unk1C;
    u8 pad20[0x28 - 0x20];
    u8 unk28;
    u8 pad29[0x3A - 0x29];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} RecordB80087FC0;

/** @brief Partial pad-context layout used by func_80087FC0. */
typedef struct
{
    u8 pad0[0x28];
    u32 unk28;
} PadCtxB80087FC0;

/** @brief Partial fixed-block layout used by func_80087FC0. */
typedef struct
{
    u8 pad0[0x13E];
    u8 unk13E;
} FixedB80087FC0;

/* ------------------------------------------------------------------ *
 * File-scope externs and forward prototypes.                         *
 * ------------------------------------------------------------------ */

/* Actor lookup helper defined later in this TU; callers view the returned
 * pointer through their own record types (pointer-type warnings are benign). */
RecB800FDF58_F32 *func_80087C9C(s32);

extern Binding D_80105880[];
extern Actor878B4 g_field_actor_slots[];
extern Resource g_field_resource_entries[];
extern s32 D_8010A018;
extern PadCtxB80087FC0 *g_pad_ctx;

long ratan2(long, long);
extern int abs(int);

void func_8006B240(s32, s32, s32);
void func_8006B4D0(u8, s32);
void func_8006B7A0(u8, s32);
void func_8008C7A8(void);

/* ------------------------------------------------------------------ *
 * Member functions (ascending address order).                        *
 * ------------------------------------------------------------------ */

/**
 * @brief Replaces the low four state flags for the selected field record.
 *
 * Resolves a field record from @p arg0, then uses its byte at offset 0x3A to
 * select a 0x23C-byte state entry. A missing record leaves the state unchanged.
 *
 * @param arg0 Selector passed to func_80087C9C.
 * @param arg1 New low-four-bit flag value.
 */
void func_80087614(s32 arg0, s32 arg1)
{
    extern State87614 D_80105AE0[];
    Record87614 *record;

    record = (Record87614 *)func_80087C9C(arg0);
    if (record != (Record87614 *)-1)
    {
        D_80105AE0[record->index].flags =
            (D_80105AE0[record->index].flags & ~0xF) | arg1;
    }
}

/**
 * @brief Update an actor record and propagate its state to the field actor table.
 * @param arg0 Actor record identifier.
 * @param arg1 Value passed to the actor setup helper.
 * @param arg2 Low flag bits stored in the actor state.
 * @param arg3 First fixed-point component.
 * @param arg4 Second fixed-point component.
 * @param arg5 Third fixed-point component.
 */
void func_80087680(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    extern State80087680 D_80105AE0[];
    Rec80087680 *rec;
    s32 flags;

    rec = (Rec80087680 *)func_80087C9C(arg0);
    if (rec != (Rec80087680 *)-1)
    {
        flags = rec->unk1C.h.hi & 3;
        func_8006B4D0(rec->unk3A, arg1);
        rec->unk25 = 0;
        rec->unk0 = arg3 << 8;
        rec->unk1C.w = (rec->unk1C.w & 0xFFFCFFFF) | (flags << 0x10);
        rec->unk4 = arg4 << 8;
        rec->unk8 = arg5 << 8;
        D_80105AE0[rec->unk3A].flags = (D_80105AE0[rec->unk3A].flags & ~0xF) | arg2;
        func_8006C3FC(rec);
    }
}

/**
 * @brief Test whether one actor faces toward another within one direction step.
 * @param first_handle Identifier of the actor whose facing direction is tested.
 * @param second_handle Identifier of the target actor.
 * @return -1 if either lookup fails, otherwise 1 when facing the target or 0.
 */
s32 func_80087770(s32 first_handle, s32 second_handle)
{
    FacingActorRecord *first;
    FacingActorRecord *second;
    s32 direction;
    s32 facing;
    s32 flags;

    first = (FacingActorRecord *)func_80087C9C(first_handle);
    if (first == (FacingActorRecord *)-1)
    {
        return -1;
    }
    second = (FacingActorRecord *)func_80087C9C(second_handle);
    if (second == (FacingActorRecord *)-1)
    {
        return -1;
    }
    flags = first->facing_flags;
    facing = (flags & 0x7F) % 5;
    if (flags & 0x80)
    {
        facing = 8 - facing;
    }
    direction = ratan2(first->position_z - second->position_z, second->position_x - first->position_x);
    direction = (direction + 0x800) / 0x200;
    direction += 2;
    direction %= 8;
    if (abs(facing - direction) < 2)
    {
        return 1;
    }
    if (abs(facing - direction + 8) < 2)
    {
        return 1;
    }
    return abs(direction - facing + 8) < 2;
}

/**
 * @brief Classify an actor record using its binding and slot state.
 * @param index Actor identifier passed to the record lookup.
 * @return -1 for a missing record, otherwise a state code from 0 through 4.
 */
s32 func_800878B4(s32 index)
{
    Record *record;
    u8 *base, *owner_base;
    Actor878B4 *actors, *actor;
    s32 offset, owner;
    record = (Record *)func_80087C9C(index);
    if (record == (Record *)-1)
    {
        return -1;
    }
    base = (u8 *)D_80105880;
    if (record->selector < 2)
    {
        offset = record->selector * 28;
    }
    else
    {
        offset = 56;
    }
    if (((Binding *)(base + offset))->state == 0)
    {
        return 0;
    }
    base = (u8 *)D_80105880;
    if (record->selector < 2)
    {
        offset = record->selector * 28;
    }
    else
    {
        offset = 56;
    }
    if (((Binding *)(base + offset))->owner != record->selector)
    {
        return 1;
    }
    owner = ((Binding *)(base + offset))->owner;
    owner_base = (u8 *)D_80105880;
    if ((u8)owner < 2)
    {
        offset = owner * 28;
    }
    else
    {
        offset = 56;
    }
    if (((Binding *)(owner_base + offset))->state == 1)
    {
        return 2;
    }
    actors = g_field_actor_slots;
    base = (u8 *)D_80105880;
    if (record->selector < 2)
    {
        offset = record->selector * 28;
    }
    else
    {
        offset = 56;
    }
    actor = (Actor878B4 *)((u8 *)actors + ((Binding *)(base + offset))->slot * 0x244);
    if (actor->first != 0)
    {
        goto return_three;
    }
    actors = g_field_actor_slots;
    base = (u8 *)D_80105880;
    if (record->selector < 2)
    {
        offset = record->selector * 28;
    }
    else
    {
        offset = 56;
    }
    actor = (Actor878B4 *)((u8 *)actors + ((Binding *)(base + offset))->slot * 0x244);
    if (actor->second == 0)
    {
        goto return_four;
    }
return_three:
    return 3;
return_four:
    return 4;
}

/**
 * @brief Reinitialize an actor resource while preserving selected state bits.
 * @param owner_id Owner identifier assigned to the actor slot.
 * @param resource_id Resource entry to initialize.
 * @param parameter_a First resource initialization parameter.
 * @param parameter_b Second resource initialization parameter.
 * @param group Group bits written to the actor slot.
 * @param x X coordinate; all three coordinates at -1 preserve the old position.
 * @param y Y coordinate.
 * @param z Z coordinate.
 * @param direction Direction byte to assign.
 * @param resource_flag Low bit assigned to the resource flags.
 * @return Initialization result, or -1 if no actor entry is available.
 */
s32 func_80087A9C(s32 owner_id, s32 resource_id, s32 parameter_a, s32 parameter_b, s32 group, s32 x,
                  s32 y, s32 z, s32 direction, s32 resource_flag)
{
    extern ActorA9C D_80105AE0[];
    extern s32 func_8006C3FC(Entry *);
    s32 position[3];
    s32 saved_state;
    Entry *entry;
    ActorA9C *actor;
    Resource *resource;
    Resource *resource_base;
    ActorA9C *actor_base;
    ActorA9C *final_base;

    entry = (Entry *)func_80087C9C(owner_id);
    if (entry == (Entry *)-1)
    {
        return -1;
    }
    position[0] = entry->unk0;
    position[1] = entry->unk4;
    position[2] = entry->unk8;
    saved_state = entry->state.half[0] & 0x1FF;
    func_8006B240(parameter_a, parameter_b, resource_id);
    func_8006B4D0(entry->unk3A, resource_id);
    func_8006B7A0(entry->unk3A, 0);
    resource_base = g_field_resource_entries;
    resource = &resource_base[resource_id];
    resource->unk10 = (s32)((resource->unk10 & ~1) | (resource_flag & 1));
    actor_base = D_80105AE0;
    actor_base[entry->unk3A].unk14 = owner_id;
    entry->unk25 = 0;
    if (x == -1 && y == x && z == y)
    {
        entry->unk0 = position[0];
        entry->unk4 = position[1];
        entry->unk8 = position[2];
    }
    else
    {
        entry->unk0 = x << 8;
        entry->unk4 = y << 8;
        entry->unk8 = z << 8;
    }
    final_base = D_80105AE0;
    actor = &final_base[entry->unk3A];
    actor->unk10 = (s32)((actor->unk10 & ~0xF) | group);
    entry->unk21 = (s8)direction;
    entry->state.word = (s32)(((s32)entry->state.word & ~0x1FF) | saved_state);
    return func_8006C3FC(entry);
}

/**
 * @brief Find an actor slot whose 0x14 field matches @p arg0.
 * @param arg0 Actor-slot lookup key.
 * @return Pointer to the matching D_800FDF58 record, or (RecB800FDF58_F32 *)-1.
 */
RecB800FDF58_F32 *func_80087C9C(s32 arg0)
{
    extern RecA80105AE0 D_80105AE0[];
    extern RecB800FDF58_F32 D_800FDF58[];
    RecA80105AE0 *ra;
    RecB800FDF58_F32 *rb;
    s32 i;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            return rb;
        }
    }
    return (RecB800FDF58_F32 *) -1;
}

/**
 * @brief Set an actor's animation byte after validating its current state.
 * @param key Actor-slot lookup key.
 * @param value Animation byte written on success.
 * @return 0 on success, or -1 when no slot matches or its state forbids it.
 */
s32 func_80087CE0(s32 key, u8 value)
{
    extern FieldActorSlotCE0 D_80105AE0[];
    extern FieldActorRecordCE0 D_800FDF58[];
    FieldActorRecordCE0 *scan;
    FieldActorRecordCE0 *found;
    FieldActorSlotCE0 *e;
    s32 i;
    s32 result;
    s16 state;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (FieldActorRecordCE0 *)-1;
check:
    if (found != (FieldActorRecordCE0 *)-1)
    {
        goto body;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
body:
    state = found->unk2A;
    if ((u16)(state - 0x93) < 2)
    {
        result = -1;
        goto done;
    }
    if (state == 0x90 || state == 0xAE || state == 0x8E)
    {
        result = -1;
        goto done;
    }
    found->unk28 = value;
    found->unk2C = 0;
    found->unk2A = 0;
    result = 0;
done:
    return result;
}

/**
 * @brief Stores a scaled position into the actor record matching @p key.
 *
 * Scans the first 13 D_80105AE0 slots for one whose 0x14 field equals @p key.
 * On a hit, writes @p x, @p y and @p z (each shifted left 8) into the parallel
 * D_800FDF58 record's first three words and returns 0; otherwise returns -1.
 *
 * @param key Actor-slot lookup key.
 * @param x X coordinate; stored shifted left 8.
 * @param y Y coordinate; stored shifted left 8.
 * @param z Z coordinate; stored shifted left 8.
 * @return 0 on success, or -1 when no slot matches.
 */
s32 func_80087D8C(s32 key, s32 x, s32 y, s32 z)
{
    extern FieldActorSlotCE0 D_80105AE0[];
    extern FieldActorRecordCE0 D_800FDF58[];
    FieldActorRecordCE0 *p = D_800FDF58;
    FieldActorSlotCE0 *e = D_80105AE0;
    FieldActorRecordCE0 *result;
    s32 i;

    i = 0;
    while (i < 13)
    {
        if (e->unk14 == key)
        {
            result = p;
            goto found;
        }
        i++;
        e++;
        p++;
    }
    result = (FieldActorRecordCE0 *)-1;
found:
    if (result == (FieldActorRecordCE0 *)-1)
    {
        return -1;
    }
    result->unk0 = x << 8;
    result->unk4 = y << 8;
    result->unk8 = z << 8;
    return 0;
}

/**
 * @brief Set an actor's animation byte and slot word after validating state.
 * @param key Actor-slot lookup key.
 * @param value Value written to the matched slot's 0x168 word.
 * @return 0 on success, or -1 when no slot matches or its state forbids it.
 */
s32 func_80087E00(s32 key, s32 value)
{
    extern FieldActorSlotE00 D_80105AE0[];
    extern FieldActorRecordE00 D_800FDF58[];
    FieldActorRecordE00 *scan;
    FieldActorRecordE00 *found;
    FieldActorSlotE00 *e;
    s32 i;
    s32 result;
    s16 state;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (FieldActorRecordE00 *)-1;
check:
    if (found != (FieldActorRecordE00 *)-1)
    {
        goto body;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
body:
    state = found->unk2A;
    if ((u16)(state - 0x93) < 2)
    {
        result = -1;
        goto done;
    }
    if (state == 0x90 || state == 0xAE || state == 0x8E)
    {
        result = -1;
        goto done;
    }
    found->unk28 = 0xFE;
    D_80105AE0[found->unk3A].unk168 = value;
    found->unk2C = 0;
    if (found->unk2A != 0x8B && found->unk2A != 0x99)
    {
        found->unk2A = 0;
    }
    result = 0;
done:
    return result;
}

/**
 * @brief Look up a value from a self-relative offset table anchored at
 *        D_8010A018.
 * @param arg0 Index into the u16 offset table.
 * @return D_8010A018 plus the u16 offset at index arg0.
 */
s32 func_80087EF0(s32 arg0)
{
    return D_8010A018 + *(u16*)((arg0 * 2) + D_8010A018);
}

/**
 * @brief Find the D_80105AE0 slot whose 0x14 field matches @p arg0.
 * @param arg0 Actor-slot lookup key.
 * @return Pointer to the matching slot, or (Struct_D80105AE0 *)-1.
 */
Struct_D80105AE0 *func_80087F0C(s32 arg0)
{
    extern Struct_D80105AE0 D_80105AE0[];
    Struct_D80105AE0 *rec;
    s32 i;

    rec = D_80105AE0;
    for (i = 0; i < 0xD; i++)
    {
        if (rec->unk14 == arg0)
        {
            return rec;
        }
        rec++;
    }
    return (Struct_D80105AE0 *) -1;
}

/**
 * @brief Copy the matched actor record's leading triple into @p arg1.
 * @param arg0 Actor-slot lookup key.
 * @param arg1 Destination triple filled on a match.
 * @return 0 on a match, or -1 when no slot matches.
 * @see decomp.me (100%)
 */
s32 func_80087F44(s32 arg0, OutRec *arg1)
{
    extern RecA80105AE0 D_80105AE0[];
    extern RecB800FDF58_F307 D_800FDF58[];
    RecA80105AE0 *ra;
    RecB800FDF58_F307 *rb;
    RecB800FDF58_F307 *found;
    s32 i;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecB800FDF58_F307 *) -1;
check:
    if (found != (RecB800FDF58_F307 *) -1)
    {
        arg1->unk0 = found->unk0;
        arg1->unk4 = found->unk4;
        arg1->unk8 = found->unk8;
        return 0;
    }
    return -1;
found_it:
    found = rb;
    goto check;
}

/**
 * @brief Find an actor by key and update its nine-bit control mode.
 * @param arg0 Actor-slot lookup key.
 * @param arg1 New mode; only its low nine bits are used.
 * @return -1 when no actor matches, or 0 after updating the record.
 */
s32 func_80087FC0(s32 arg0, s32 arg1)
{
    extern StateB80087FC0 D_80105AE0[];
    extern RecordB80087FC0 D_800FDF58[];
    StateB80087FC0 *ra;
    RecordB80087FC0 *rb;
    RecordB80087FC0 *found;
    s32 i;
    s32 masked;
    u8 unk3a;
    FixedB80087FC0 *fixed = (FixedB80087FC0 *) 0x801ED600;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecordB80087FC0 *) -1;
check:
    if (found != (RecordB80087FC0 *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    found->unk1C = (found->unk1C & ~0x1FF) | (arg1 & 0x1FF);
    masked = (u16) found->unk1C & 0x1FF;
    if (masked != 1)
    {
        if (masked < 2)
        {
            if (masked == 0)
            {
                unk3a = found->unk3A;
                found->unk28 = 0xFF;
                found->unk10 = 0;
                if (unk3a == 1)
                {
                    if (!(g_pad_ctx->unk28 & 1))
                    {
                        goto zero_flag;
                    }
                    fixed->unk13E = unk3a;
                }
            }
        }
    }
    else
    {
        found->unk28 = 0xFF;
        found->unk10 = 0;
        func_8008C7A8();
        if (found->unk3A == masked)
        {
        zero_flag:
            fixed->unk13E = 0;
        }
    }
    return 0;
}
