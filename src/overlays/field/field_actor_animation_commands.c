/** @file field_actor_animation_commands.c
 * @brief Key-based animation commands, actor slot queries, facing changes, and pending animation restarts.
 */

/* func_8008AD44 */
#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0;

/**
 * @brief Parallel per-actor record; array element stride 0x54.
 */
typedef struct
{
    u8 data[0x54];
} Struct_D800FDF58;

extern Struct_D80105AE0 D_80105AE0[];
extern Struct_D800FDF58 D_800FDF58[];
void func_8008BE38(Struct_D800FDF58 *, s32);

/**
 * @brief Finds the actor slot matching @p key and activates its record.
 *
 * Scans the first 13 D_80105AE0 slots for one whose 0x14 field equals @p key.
 * On a hit, activates the parallel D_800FDF58 record (func_8008BE38 with 1) and
 * returns 0; if no slot matches, returns -1.
 */
s32 func_8008AD44(s32 key)
{
    Struct_D800FDF58 *p = D_800FDF58;
    Struct_D80105AE0 *e = D_80105AE0;
    s32 i;

    i = 0;
    while (i < 13)
    {
        if (e->unk14 == key)
        {
            goto found;
        }
        i++;
        e++;
        p++;
    }
    p = (Struct_D800FDF58 *)-1;
found:
    if (p == (Struct_D800FDF58 *)-1)
    {
        return -1;
    }
    func_8008BE38(p, 1);
    return 0;
}


/* field254 */
#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} RecA80105AE0;

typedef struct
{
    u8 pad0[0x21];
    u8 unk21; /* 0x21 */
    u8 pad22[0x54 - 0x22];
} RecB800FDF58;




s32 func_8008ADB4(s32 arg0)
{
    RecA80105AE0 *ra;
    RecB800FDF58 *rb;
    RecB800FDF58 *found;
    s32 i;

    rb = ((RecB800FDF58 *)D_800FDF58);
    ra = ((RecA80105AE0 *)D_80105AE0);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecB800FDF58 *) -1;
check:
    if (found != (RecB800FDF58 *) -1)
    {
        return found->unk21 & 0x7F;
    }
    return -1;
found_it:
    found = rb;
    goto check;
}


/* field_queue_actor_animation_by_handle */
/**
 * @file field_queue_actor_animation_by_handle.c
 * @brief Resolve a runtime field-actor handle and queue an animation request.
 */

#include "common.h"

/** @brief Field object record paired with a runtime actor slot. */
typedef struct
{
    u8 _pad000[0x3A];
    u8 object_index;
    u8 _pad03B[0x54 - 0x3B];
} FieldObjectRecord;

/** @brief Runtime field-actor slot used to resolve actor handles. */
typedef struct
{
    u8 _pad000[0x14];
    s32 handle;
    u8 _pad018[0x23C - 0x18];
} FieldActorSlot;

/** @brief Partial view of the shared context referenced by g_pad_ctx. */
typedef struct
{
    u8 _pad000[0x3158];
    s32 unk3158;
} FieldPadContext;



extern FieldPadContext *g_pad_ctx;

extern void func_8008C024(FieldObjectRecord *object, s32 animation_id);

/**
 * @brief Queue an animation for the field actor identified by a runtime handle.
 * @param actor_handle Handle to locate in the first 13 runtime actor slots.
 * @param animation_id Animation identifier forwarded to the actor update path.
 * @return 0 when the actor is found and updated, or -1 when no actor matches.
 */
s32 func_8008AE14(s32 actor_handle, s32 animation_id)
{
    FieldObjectRecord *object;
    FieldActorSlot *actor_slot;
    s32 context_counter;
    s32 actor_index;

    object = ((FieldObjectRecord *)D_800FDF58);
    actor_slot = ((FieldActorSlot *)D_80105AE0);
    actor_index = 0;
scan_actor:
    if (actor_slot->handle != actor_handle)
    {
        actor_slot += 1;
        actor_index += 1;
        object += 1;
        if (actor_index >= 13)
        {
            object = (FieldObjectRecord *)-1;
        }
        else
        {
            goto scan_actor;
        }
    }

    if (object == (FieldObjectRecord *)-1)
    {
        return -1;
    }

    if (object->object_index >= 3)
    {
        context_counter = g_pad_ctx->unk3158;
        if (context_counter != -1)
        {
            g_pad_ctx->unk3158 = context_counter + 1;
        }
    }

    func_8008C024(object, animation_id);
    return 0;
}


/* func_8008AEB0 */
#include "common.h"

typedef struct { u8 pad0[0x14]; s32 unk14; u8 pad18[0x224]; } ActorAEB0;
typedef struct { u8 pad0[0x1C]; u32 unk1C; u8 pad20[0xA]; s16 unk2A; u8 pad2C[0x28]; } RecAEB0;



s32 func_8008AEB0(s32 id)
{
    s32 v;
    s32 result;
    s32 masked;
    RecAEB0 *rec;
    ActorAEB0 *actor;

    rec = ((RecAEB0 *)D_800FDF58);
    actor = ((ActorAEB0 *)D_80105AE0);
    v = 0;
    while (v < 13) {
        if (actor->unk14 == id) {
            v = (s32)rec;
            goto found_done;
        }
        v++;
        actor++;
        rec++;
    }
    v = -1;
found_done:
    if (v == -1) {
        return -1;
    }
    if (((*(u16 *)&((RecAEB0 *)v)->unk1C) & 0x1FF) < 2) {
        result = 0;
        if (((RecAEB0 *)v)->unk2A == 0) {
            masked = ((RecAEB0 *)v)->unk1C & 0x600;
            result = masked == 0;
        }
    } else {
        result = 0;
        if ((((RecAEB0 *)v)->unk2A == 0x81) || (((RecAEB0 *)v)->unk2A == 0)) {
            result = 1;
        }
    }
    return result;
}


/* func_8008AF68 */
#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} CommandView5_Struct_D80105AE0;

/**
 * @brief Parallel per-actor record; array element stride 0x54.
 */
typedef struct
{
    u8 data[0x54];
} CommandView5_Struct_D800FDF58;



void func_8008BF88();

/**
 * @brief Finds the actor slot matching @p key and notifies its record.
 *
 * Scans the first 13 ((CommandView5_Struct_D80105AE0 *)D_80105AE0) slots for one whose 0x14 field equals @p key;
 * on a hit, forwards the parallel ((CommandView5_Struct_D800FDF58 *)D_800FDF58) record and the remaining caller
 * arguments to func_8008BF88 and returns 0, otherwise returns -1.
 *
 * @note gcc272_cdk, 100% match. Preserving arg1-arg3 through the scan is what
 *       keeps the loop key/record temporaries in t0/t1 as in the target.
 */
s32 func_8008AF68(s32 key, s32 arg1, s32 arg2, s32 arg3)
{
    CommandView5_Struct_D800FDF58 *p = ((CommandView5_Struct_D800FDF58 *)D_800FDF58);
    CommandView5_Struct_D80105AE0 *e = ((CommandView5_Struct_D80105AE0 *)D_80105AE0);
    s32 i;

    i = 0;
    while (i < 13)
    {
        if (e->unk14 == key)
        {
            goto found;
        }
        i++;
        e++;
        p++;
    }
    p = (CommandView5_Struct_D800FDF58 *)-1;
found:
    if (p == (CommandView5_Struct_D800FDF58 *)-1)
    {
        return -1;
    }
    func_8008BF88(p, arg1, arg2, arg3);
    return 0;
}


/* func_8008AFD8 */
#include "common.h"

/** @brief Actor position, direction, and animation fields in the 0x54-byte entry. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x21 - 0xC];
    u8 unk21;
    u8 pad22[2];
    s8 unk24;
    u8 pad25[2];
    s8 unk27;
    u8 pad28[6];
    s16 unk2E;
    u8 pad30[10];
    u8 unk3A;
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} Entry;
/** @brief Actor identity and movement flags in the 0x23C-byte record. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x174 - 0x18];
    s32 unk174;
    u8 pad178[0x23C - 0x178];
} Actor;
/** @brief Resource flags selecting the direction mode. */
typedef struct
{
    u8 pad0[0x10];
    s32 unk10;
} Resource;


extern Resource g_field_resource_entries[];
extern s32 ratan2(s32 y, s32 x);
void field_restart_actor_animation();
/**
 * @brief Turn the source actor toward the target and reset its movement state.
 * @param source_id Source actor identifier.
 * @param target_id Target actor identifier.
 * @return Zero on success, or -1 if either actor is absent.
 */
s32 func_8008AFD8(s32 source_id, s32 target_id)
{
    Entry *first;
    Entry *second;
    Entry *entry;
    Actor *actor;
    Entry *entry2;
    Actor *actor2;
    Actor *base;
    Actor *slot;
    s32 i;
    s32 angle;

    entry = ((Entry *)D_800FDF58);
    actor = ((Actor *)D_80105AE0);
    for (i = 0; i < 13; i++, actor++, entry++)
    {
        if (actor->unk14 == source_id)
        {
            goto first_found;
        }
    }
    first = (Entry *)-1;
first_check:
    if (first != (Entry *)-1)
    {
        goto second_start;
    }
    return -1;
first_found:
    first = entry;
    goto first_check;
second_found:
    second = entry2;
    goto second_check;
second_start:
    entry2 = ((Entry *)D_800FDF58);
    actor2 = ((Actor *)D_80105AE0);
    for (i = 0; i < 13; i++, actor2++, entry2++)
    {
        if (actor2->unk14 == target_id)
        {
            goto second_found;
        }
    }
    second = (Entry *)-1;
second_check:
    if (second == (Entry *)-1)
    {
        return -1;
    }

    angle = ratan2(first->unk8 - second->unk8, second->unk0 - first->unk0);
    if (!(g_field_resource_entries[first->unk3B].unk10 & 1))
    {
        if (angle < -0x700)
        {
            first->unk21 = 2;
        }
        else if (angle < -0x500)
        {
            first->unk21 = 3;
        }
        else if (angle < -0x300)
        {
            first->unk21 = 4;
        }
        else if (angle < -0x100)
        {
            first->unk21 = 0x83;
        }
        else if (angle < 0x100)
        {
            first->unk21 = 0x82;
        }
        else if (angle < 0x300)
        {
            first->unk21 = 0x81;
        }
        else if (angle < 0x500)
        {
            first->unk21 = 0;
        }
        else if (angle < 0x700)
        {
            first->unk21 = 1;
        }
        else
        {
            first->unk21 = 2;
        }
    }
    else
    {
        if ((u32)(angle - 0x401) < 0x7FFU)
        {
            first->unk21 = 0;
        }
        else if (angle < -0x400)
        {
            if (angle < -0xBFF)
            {
                first->unk21 = 0x80;
            }
            else
            {
                first->unk21 = 0;
            }
        }
        else
        {
            first->unk21 = 0x80;
        }
    }

    base = ((Actor *)D_80105AE0);
    first->unk2E = 1;
    first->unk27 = 0;
    first->unk24 = 1;
    slot = &base[first->unk3A];
    slot->unk174 &= ~0x1800;
    field_restart_actor_animation(first);
    return 0;
}


/* func_8008B1C8 */
#include "common.h"

typedef struct {
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} ActorSlot;

typedef struct {
    u8 pad0[0x21];
    u8 unk21;
    u8 pad22[2];
    s8 unk24;
    u8 pad25[2];
    s8 unk27;
    u8 pad28[6];
    s16 unk2E;
    u8 pad30[0xA];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} CommandView7_Entry;

typedef struct {
    u8 pad0[0x174];
    s32 unk174;
    u8 pad178[0x23C - 0x178];
} ActorRecord;



void field_restart_actor_animation();

s32 func_8008B1C8(s32 arg0, u8 arg1)
{
    ActorSlot *ra;
    CommandView7_Entry *rb;
    CommandView7_Entry *found;
    s32 i;
    ActorRecord *base;
    ActorRecord *slot;

    rb = ((CommandView7_Entry *)D_800FDF58);
    ra = ((ActorSlot *)D_80105AE0);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (CommandView7_Entry *)-1;
check:
    if (found == (CommandView7_Entry *)-1)
    {
        goto fail;
    }
    base = (ActorRecord *)((ActorSlot *)D_80105AE0);
    found->unk21 = arg1;
    found->unk2E = 1;
    found->unk27 = 0;
    found->unk24 = 1;
    slot = &base[found->unk3A];
    slot->unk174 &= ~0x1800;
    field_restart_actor_animation(found);
    return 0;
found_it:
    found = rb;
    goto check;
fail:
    return -1;
}


/* func_8008B288 */
#include "common.h"

/** @brief Per-actor slot in ((SlotA *)D_80105AE0); stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} SlotA;

/** @brief Per-actor record in ((EntryB *)D_800FDF58); stride 0x54. */
typedef struct
{
    u8 pad0[0x21];
    u8 unk21;
    u8 pad22[0x3B - 0x22];
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} EntryB;

typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    u8 padA[0xE - 0xA];
    s16 unkE;
    u32 flags;
} FieldResourceEntry;





/**
 * @brief Derive a control value for the actor slot matching the requested key.
 * @param arg0 Actor-slot lookup key.
 * @return Derived control value, or -1 when no matching actor slot exists.
 */
s32 func_8008B288(s32 arg0)
{
    EntryB* scan;
    EntryB* found;
    SlotA* e;
    s32 i;
    s32 result;
    s32 state;

    scan = ((EntryB *)D_800FDF58);
    e = ((SlotA *)D_80105AE0);
    i = 0;
loop:
    i++;
    if (e->unk14 == arg0)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (EntryB*)-1;
check:
    if (found != (EntryB*)-1)
    {
        goto lookup;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
lookup:
    if (((FieldResourceEntry *)g_field_resource_entries)[found->unk3B].flags & 1)
    {
        goto special;
    }
    {
        state = found->unk21;
        if ((state & 0x7F) < 0xF)
        {
            switch (found->unk21)
            {
            case 1:
            case 6:
            case 11:
                result = 0x60;
                break;
            case 2:
            case 7:
            case 12:
                result = 0x80;
                break;
            case 3:
            case 8:
            case 13:
                result = 0xA0;
                break;
            case 4:
            case 9:
            case 14:
                result = 0xC0;
                break;
            case 129:
            case 134:
            case 139:
                result = 0x20;
                break;
            case 130:
            case 135:
            case 140:
                result = 0;
                break;
            case 131:
            case 136:
            case 141:
                result = 0xE0;
                break;
            case 132:
            case 137:
            case 142:
                result = 0xC0;
                break;
            case 0:
            case 5:
            case 10:
            case 128:
            case 133:
            case 138:
            default:
                result = 0x40;
                break;
            }
        }
        else
        {
            result = 0x40;
        }
    }
done:
    return result;
special:
    result = found->unk21 & 0x80;
    goto done;
}


/* field_actor_slot_lookup */
#include "common.h"

/*
 * Actor slot lookups keyed by the word at 0x14 of ((CommandView9_SlotA *)D_80105AE0). The first 13
 * slots are scanned in parallel with the ((CommandView9_EntryB *)D_800FDF58) records, so a hit in one
 * array selects the same index in the other.
 */

/** @brief Per-actor slot in ((CommandView9_SlotA *)D_80105AE0); stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;   /* 0x14 lookup key */
    u8 pad18[0x224];
} CommandView9_SlotA;

/** @brief Per-actor record in ((CommandView9_EntryB *)D_800FDF58); stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;    /* 0x3A object index / track selector */
    u8 pad3B[0x19];
} CommandView9_EntryB;

s32 func_800839F8();
s32 func_80083EEC(u8 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation();



extern s32 D_80105880[];

/**
 * @brief Finds the track value associated with the actor slot matching @p key.
 *
 * Scans the first 13 actor slots in parallel with ((CommandView9_EntryB *)D_800FDF58). On a hit, the
 * record's track selector at 0x3A chooses one of the three 0x1C-byte entries
 * in D_80105880; selectors >= 2 clamp to the third entry.
 *
 * @param key Value compared against each slot's unk14.
 * @return The selected D_80105880 word, or -1 when no slot matches.
 */
s32 func_8008B398(s32 key)
{
    CommandView9_EntryB *scan;
    CommandView9_EntryB *found;
    CommandView9_SlotA *e;
    s32 i;
    s32 offset;
    s32 result;
    u8 *base;

    scan = ((CommandView9_EntryB *)D_800FDF58);
    e = ((CommandView9_SlotA *)D_80105AE0);
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
    found = (CommandView9_EntryB *)-1;
check:
    if (found != (CommandView9_EntryB *)-1)
    {
        goto lookup;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
lookup:
    base = (u8 *)D_80105880;
    if (found->unk3A < 2)
    {
        offset = found->unk3A * 0x1C;
    }
    else
    {
        offset = 0x38;
    }
    result = *(s32 *)(base + offset);
done:
    return result;
}

/**
 * @brief Start an animation on the actor whose slot key matches, resolving it through the record's own index.
 * @param arg0 Slot key compared against unk14.
 * @param arg1 Forwarded to func_80083EEC as its third argument.
 * @return -1 when no slot matches, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B42C(s32 arg0, s32 arg1)
{
    CommandView9_SlotA *ra;
    CommandView9_EntryB *rb;
    CommandView9_EntryB *found;
    s32 i;
    s32 anim;

    rb = ((CommandView9_EntryB *)D_800FDF58);
    ra = ((CommandView9_SlotA *)D_80105AE0);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (CommandView9_EntryB *) -1;
check:
    if (found != (CommandView9_EntryB *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    anim = func_800839F8(found->unk3A, 0, rb);
    if ((anim != -1) && (func_80083EEC(found->unk3A, anim, arg1) != 0))
    {
        field_start_actor_animation(anim, 0, 0);
        return 0;
    }
    return 1;
}

/**
 * @brief Start an animation on the actor whose slot key matches, resolving it with index 0.
 * @param arg0 Slot key compared against unk14.
 * @param arg1 Forwarded to func_80083EEC as its third argument.
 * @return -1 when no slot matches, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B500(s32 arg0, s32 arg1)
{
    CommandView9_SlotA *ra;
    CommandView9_EntryB *rb;
    CommandView9_EntryB *found;
    s32 i;
    s32 anim;

    rb = ((CommandView9_EntryB *)D_800FDF58);
    ra = ((CommandView9_SlotA *)D_80105AE0);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (CommandView9_EntryB *) -1;
check:
    if (found != (CommandView9_EntryB *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    anim = func_800839F8(0, 0, rb);
    if ((anim != -1) && (func_80083EEC(found->unk3A, anim, arg1) != 0))
    {
        field_start_actor_animation(anim, 0, 0);
        return 0;
    }
    return 1;
}


/* func_8008B5D0 */
#include "common.h"

/** @brief Actor-slot lookup entry from ((FieldActorSlotEntry *)D_80105AE0). */
typedef struct
{
    u8 pad0[0x14];
    s32 lookup_key;
    u8 pad18[0x224];
} FieldActorSlotEntry;

/** @brief Runtime actor record from ((FieldActorRecord *)D_800FDF58). */
typedef struct
{
    u8 pad0[0x3A];
    u8 track_selector;
    u8 pad3B[0x19];
} FieldActorRecord;

s32 func_800839F8();
s32 func_80083EEC(u8 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation();




/**
 * @brief Collect matching actor selectors and start the requested actor animation.
 * @param lookup_key Value matched against the first 13 actor-slot entries.
 * @param animation_param Value forwarded to func_80083EEC.
 * @param repeat_count Number of repeated lookup passes used to collect target selectors.
 * @param unused_target Caller-provided target pointer; this routine does not consume it.
 * @return -1 if no actor record matches, 0 if the animation starts, or 1 if setup fails.
 */
s32 func_8008B5D0(s32 lookup_key, s32 animation_param, s32 repeat_count, s32 *unused_target)
{
    FieldActorSlotEntry *slot;
    FieldActorRecord *scan_record;
    FieldActorRecord *final_record;
    FieldActorRecord *found;
    s32 i;
    s32 repeat_index;
    s32 target_count;
    s32 animation_slot;
    s32 targets[16];

    repeat_index = 0;
    target_count = repeat_index;
    for (; repeat_index < repeat_count; repeat_index++)
    {
        scan_record = ((FieldActorRecord *)D_800FDF58);
        slot = ((FieldActorSlotEntry *)D_80105AE0);
        for (i = 0; i < 13; i++, slot++, scan_record++)
        {
            if (slot->lookup_key == lookup_key)
            {
                found = scan_record;
                goto matched;
            }
        }
        found = (FieldActorRecord *)-1;
    matched:
        if (found != (FieldActorRecord *)-1)
        {
            targets[target_count] = found->track_selector;
            target_count++;
        }
    }

    final_record = ((FieldActorRecord *)D_800FDF58);
    slot = ((FieldActorSlotEntry *)D_80105AE0);
    for (i = 0; i < 13; i++, slot++, final_record++)
    {
        if (slot->lookup_key == lookup_key)
        {
            goto found_it;
        }
    }
    found = (FieldActorRecord *)-1;
check:
    if (found != (FieldActorRecord *)-1)
    {
        goto body;
    }
    return -1;
found_it:
    found = final_record;
    goto check;
body:
    animation_slot = func_800839F8(found->track_selector, 0);
    if ((animation_slot != -1) && (func_80083EEC(found->track_selector, animation_slot, animation_param) != 0))
    {
        field_start_actor_animation(animation_slot, target_count, (u8 *)targets);
        return 0;
    }
    return 1;
}


/* field36 */
#include "common.h"

typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
} UnkStruct_8008B724;

extern s32 D_8010A020[];

void func_8008B724(void)
{
    D_8010A020[2] = 0;
    D_8010A020[1] = 0;
    D_8010A020[0] = 0;
}


/* func_8008B73C */
#include "common.h"

extern s32 D_8010A020[];

extern u8 g_field_actor_slots[];


void field_start_actor_animation();

/**
 * @brief Restart animations for three pending actor slots and clear their update flags.
 * @see decomp.me (100%)
 */
void func_8008B73C(void)
{
    s32 i;
    s32 *flag;
    u8 *rec;
    s32 scaled_index;
    s32 index;
    u8 *slot;
    s32 slot_offset;
    u8 *actor_base;
    u8 *ae0_base;

    i = 0;
    actor_base = g_field_actor_slots;
    ae0_base = ((u8 *)D_80105AE0);
    flag = D_8010A020;
    rec = ((u8 *)D_80105880);
restart_slots:
    {
        if (*flag != 0 && *(s32 *)(rec + 0x0) == 2)
        {
            slot_offset = *(s32 *)(rec + 0x18) * 0x244;
            *(s32 *)(slot_offset + (u32)actor_base + 0xC) = *(s32 *)(slot_offset + (u32)actor_base + 0x10);
            field_start_actor_animation(*(s32 *)(rec + 0x18), 0, 0);
            index = *(s32 *)(rec + 0xC);
            if (index >= 3)
            {
                index = 2;
            }
            scaled_index = index * 8;
            (ae0_base + (((scaled_index + index) * 0x10) - index) * 4)[0x179] = *(u8 *)(rec + 0x18);
            *flag = 0;
            slot = (u8 *)(*(s32 *)(rec + 0x18) * 0x244 + (u32)actor_base);
            slot[0x2A] = 1;
        }
        flag += 1;
        i += 1;
        rec += 0x1C;
    }
    if (i < 3)
    {
        goto restart_slots;
    }
}
