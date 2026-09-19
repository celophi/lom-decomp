#include "common.h"

/**
 * @file field_actor_reactions.c
 * @brief Actor reaction state transitions: knockback/stun selection, pending
 *        draw-flag clears, animation re-arming, and depth-overlap eligibility.
 *
 * Groups the eleven reaction routines in 8008B870..8008C728. They share the
 * actor-slot table D_80105AE0 (0x23C stride), the owner table D_800FD818
 * (0x268 stride), the actor record table D_800FDF58 (0x54 stride), and the
 * animation-resource block D_800FB3C8 (0x244 stride).
 */

/**
 * @brief Unified view of a D_80105AE0 actor-slot record (0x23C stride).
 *
 * Only the fields accessed directly through @c D_80105AE0[i] are modelled;
 * functions that walk the table through a local pointer keep their own view.
 */
typedef struct
{
    u8 pad0[8];
    s32 value;              /* 0x08 */
    u32 unkC;               /* 0x0C */
    u8 pad10[0x12E - 0x10];
    u16 extent;             /* 0x12E */
    u8 pad130[0x16C - 0x130];
    s8 unk16C;              /* 0x16C */
    u8 pad16D[0x170 - 0x16D];
    u8 unk170;              /* 0x170 */
    u8 pad171[0x174 - 0x171];
    u32 unk174;             /* 0x174 */
    u32 unk178;             /* 0x178 */
    u8 pad17C[0x18D - 0x17C];
    u8 unk18D;              /* 0x18D */
    u8 pad18E[0x23C - 0x18E];
} ReactionSlot;

/** @brief Unified view of a D_800FD818 owner record (0x268 stride). */
typedef struct
{
    u8 pad0;
    u8 unk1;                /* 0x01 */
    u8 pad2[0x259 - 2];
    u8 state;               /* 0x259 */
    u8 pad25A[0x260 - 0x25A];
    s16 unk260;             /* 0x260 */
    u8 pad262[0x268 - 0x262];
} FieldReactionOwner;

/** @brief Opaque D_800FDF58 actor record (0x54 stride); walked via local views. */
typedef struct
{
    u8 pad[0x54];
} ActorSlot54;

/* ----- func_8008B870 local views ----- */
/** @brief Position, state, timers, and slot in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y;
    u8 pad8[0x19];
    u8 state;
    u8 pad22[2];
    u8 active;
    u8 pad25[2];
    u8 unknown27;
    u8 pad28[2];
    s16 value;
    u8 pad2c[2];
    u16 timer;
    u16 counter;
    u8 pad32[5];
    s8 height;
    u8 pad38[2];
    u8 slot;
    u8 tail[0x19];
} f870_FieldReactionActor;
/** @brief Active state and actor index in a 0x1C-byte selection record. */
typedef struct
{
    s32 active;
    u8 pad4[8];
    s32 actor_index;
    u8 tail[0xC];
} f870_FieldReactionSelection;

/* ----- func_8008BC5C (field363) local views ----- */
/** @brief Caller struct; only the actor slot index at 0x3A is read. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
} f363_ArgStruct;
/** @brief Actor slot record view used by func_8008BC5C. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 padC[0x170 - 0x10];
    u8 unk170;
    u8 pad171[3];
    s32 unk174;
    u32 unk178;
    u8 pad17C[0x23C - 0x17C];
} f363_Record;

/* ----- func_8008BCF8 (field297) local views ----- */
/** @brief Caller struct; only the actor slot index at 0x3A is read. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} f297_Rec;

/* ----- func_8008BD88 local views ----- */
/** @brief Actor slot record view keyed on the animation id at 0x14. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} fBD88_Struct_D80105AE0;
/** @brief Actor record view reset when a match is found. */
typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[0x28 - 0x12];
    u8 unk28;
    u8 pad29[0x2C - 0x29];
    u16 unk2C;
    u8 pad2E[0x3A - 0x2E];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} fBD88_Struct_D800FDF58;

/* ----- func_8008BE38 local views ----- */
/** @brief Actor record reset and re-armed by func_8008BE38. */
typedef struct
{
    u8 pad0[0x4];
    s32 unk4;
    u8 pad8[0x1C - 0x8];
    s32 unk1C;
    u8 pad20[0x21 - 0x20];
    u8 unk21;
    u8 pad22[0x24 - 0x22];
    u8 unk24;
    u8 pad25[0x27 - 0x25];
    u8 unk27;
    u8 unk28;
    u8 pad29[0x2A - 0x29];
    s16 unk2A;
    u8 pad2C[0x2E - 0x2C];
    u16 unk2E;
    u8 pad30[0x3A - 0x30];
    u8 unk3A;
} fBE38_FieldActorRecord;
/** @brief Animation-slot flag view used by func_8008BE38. */
typedef struct
{
    u8 pad0[0x174];
    u32 unk174;
    u8 pad178[0x23C - 0x178];
} fBE38_FieldAnimationRecord;

/* ----- func_8008BF88 (field364) local views ----- */
/** @brief Actor slot record view used by func_8008BF88. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 padC[0x170 - 0x10];
    u8 unk170;
    u8 pad171[3];
    s32 unk174;
    u32 unk178;
    u8 pad17C[0x23C - 0x17C];
} f364_Record;
/** @brief Actor state struct operated on when arming an animation. */
typedef struct
{
    u8 pad0[0x1B];
    s8 unk1B;
    u8 pad1C[0x20 - 0x1C];
    s8 unk20;
    u8 unk21;
    u8 pad22[0x24 - 0x22];
    s8 unk24;
    u8 pad25[0x27 - 0x25];
    s8 unk27;
    u8 pad28[0x2A - 0x28];
    s16 unk2A;
    u8 pad2C[0x2E - 0x2C];
    s16 unk2E;
    u8 pad30[0x3A - 0x30];
    u8 unk3A;
} f364_ArgStruct;

/* ----- func_8008C024 local views ----- */
/** @brief Entry record acted on by func_8008C024. */
typedef struct
{
    u8 pad0[0x20];
    u8 unk20;
    u8 pad21[0x2A - 0x21];
    s16 unk2A;
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} fC024_EntryA0;

/* ----- func_8008C104 local views ----- */
/** @brief Actor state and animation fields reset by this transition. */
typedef struct
{
    u32 pad0, word4;
    u8 pad8[0x14];
    u32 flags;
    u8 pad20, state, pad22[2], active, pad25[2], byte27, byte28, pad29;
    s16 animation;
    u16 pad_2c, value;
    u8 pad30[10], selector, resource;
} fC104_Actor;
/** @brief Actor slot flags with the original 0x23C stride (byte read at 0x178). */
typedef struct
{
    u8 pad[0xC];
    u32 flags;
    u8 pad10[0x164];
    u32 flags174;
    u8 flags178;
    u8 tail[0xC3];
} fC104_Record;
/** @brief Resource descriptor containing animation eligibility flags. */
typedef struct
{
    u8 pad[0xE];
    u16 flags;
    u8 tail[4];
} fC104_Resource;

/* ----- func_8008C2EC local views ----- */
/** @brief Actor depth, presence, and slot index in a 0x54-byte record. */
typedef struct
{
    u8 pad[8];
    s32 z;
    u8 pad_c[0x19];
    u8 presence;
    u8 pad26[0x14];
    u8 slot_index;
    u8 pad3B[0x19];
} fC2EC_Actor;
/** @brief Actor-slot identity, eligibility flags, and depth extent. */
typedef struct
{
    s32 pad0, active;
    u8 pad8[8];
    s32 group_flags, id;
    u8 pad18[0x116];
    u16 extent;
    u8 pad130[0x10C];
} fC2EC_Slot;

/* ----- func_8008C4A8 local views ----- */
/** @brief Actor slot retry/threshold view used by func_8008C4A8. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x14 - 0x10];
    s32 unk14;
    u8 pad18[0x18D - 0x18];
    u8 unk18D;
    u8 pad18E[0x23C - 0x18E];
} fC4A8_FieldSlotState;
/** @brief Actor record view walked when matching an animation key. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} fC4A8_FieldActorState;
/** @brief Owner-type index view used by func_8008C4A8. */
typedef struct
{
    u8 pad0;
    u8 unk1;
    u8 pad2[0x268 - 2];
} fC4A8_FieldActorTypeState;

/* ----- func_8008C620 local views ----- */
/** @brief Partial FieldActorState layout used by func_8008C620. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 pad1B[1];
    s32 unk1C;
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 pad26[1];
    u8 unk27;
    u8 unk28;
    u8 pad29[1];
    s16 unk2A;
    s16 unk2C;
    u16 unk2E;
    s16 unk30;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 pad39[1];
    u8 unk3A;
    u8 unk3B;
    u32 unk3C;
    s32 unk40;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 pad50[4];
} fC620_FieldActorState;

/* ----- shared globals ----- */
extern ReactionSlot D_80105AE0[];
extern FieldReactionOwner D_800FD818[];
extern ActorSlot54 D_800FDF58[];
extern u8 D_800FB3C8[];
extern f870_FieldReactionSelection D_80105880[];
extern fC104_Resource g_field_resource_entries[];
extern u8 D_800EB068[];
extern s32 D_8010A000;
extern s32 D_800FE754;
extern s32 D_8010D020;

/* ----- forward declarations (only functions called before their definition) -----
 * func_8008BC5C uses a K&R definition below so no prototype is created; that
 * preserves func_8008C104's original zero-argument call (which relies on the
 * incoming a0 rather than setting up the argument). */
void func_8008BC5C();
void func_8008C620();

extern void field_restart_actor_animation();
extern void field_restart_actor_animation_reverse();
extern void func_800952DC();
extern void field_stop_actor_animations_for_object();
extern void func_800A2DD8();
extern void func_80084424();
extern void func_80083BC0();
extern void field_start_actor_animation();
extern s32 func_80083EEC();
extern s32 func_800839F8();
extern s32 rand(void);

/**
 * @brief Set an actor reaction state and reset its motion and animation flags.
 * @param actor Actor receiving the reaction, subject to selection and state checks.
 * @param alternate Nonzero selects state 0x0B; zero randomly selects 0x14 or 0x15.
 * @note Temporarily remove the signed height offset while changing animations.
 */
void func_8008B870(f870_FieldReactionActor *actor, s32 alternate)
{
    s32 offset;
    u16 value;
    s32 flags;
    ReactionSlot *slot;
    ReactionSlot *slots;
    f870_FieldReactionSelection *selection;
    f870_FieldReactionSelection *actor_selection;

    if (actor->slot < 3U)
    {
        D_800FD818[actor->slot].state = 5;
    }
    else if (D_80105AE0[actor->slot].value < 0)
    {
        D_8010A000 = 5;
    }
    if (!((D_80105AE0[actor->slot].unk178 >> 6) & 1))
    {
        selection = D_80105880;
        if (actor->slot < 2U)
        {
            offset = actor->slot * 0x1C;
        }
        else
        {
            offset = 0x38;
        }
        if (((f870_FieldReactionSelection *)((u8 *)selection + offset))->active != 0)
        {
            actor_selection = D_80105880;
            if (actor->slot < 2U)
            {
                offset = actor->slot * 0x1C;
            }
            else
            {
                offset = 0x38;
            }
            if (((f870_FieldReactionSelection *)((u8 *)actor_selection + offset))->actor_index == actor->slot)
            {
                return;
            }
        }
    }
    value = actor->value;
    if ((u32)(value - 0x93) >= 2U && (s16)value != 0x90)
    {
        slots = D_80105AE0;
        slot = &slots[actor->slot];
        flags = slot->unkC;
        if (!(flags & 0x200))
        {
            if ((*(u8 *)&slot->unk178 & 1) || (flags & 0x23E4))
            {
                actor->timer = 1;
                actor->unknown27 = 0;
                actor->active = 1;
                actor->state &= 0x80;
                field_restart_actor_animation(actor);
                actor->value = 0x82;
                return;
            }
            if ((actor->state & 0x7F) != 0x44)
            {
                slot->unk178 &= ~0x40;
                func_800952DC(actor, 0);
                actor->value = 0x82;
                actor->y -= actor->height << 8;
                D_80105AE0[actor->slot].unk18D = 0;
                actor->counter = 0;
                if (alternate != 0)
                {
                    actor->state = (actor->state & 0x80) + 0xB;
                }
                else
                {
                    actor->state = (actor->state & 0x80) + 0x14;
                    actor->state += rand() & 1;
                }
                func_8008BC5C(actor);
                actor->timer = 1;
                actor->active = 1;
                actor->unknown27 = 0;
                D_80105AE0[actor->slot].unk174 &= ~0x1800;
                field_restart_actor_animation(actor);
                actor->y += actor->height << 8;
                if (actor->y > 0)
                {
                    actor->y = 0;
                }
                D_80105AE0[actor->slot].unkC &= ~0x4000;
                D_80105AE0[actor->slot].unkC &= 0xFFFF7FFF;
                field_stop_actor_animations_for_object(actor, 0);
                if (actor->slot < 2U)
                {
                    func_800A2DD8(actor->slot);
                    D_80105AE0[actor->slot].unk18D = 0;
                    actor->counter = 0;
                }
            }
        }
    }
}

/**
 * @brief Clear a pending flag and its linked slot's draw bit for an actor.
 * @param arg0 Caller struct holding the actor slot index at @c unk3A.
 */
void func_8008BC5C(arg0)
f363_ArgStruct *arg0;
{
    f363_Record *base = D_80105AE0;
    f363_Record *temp_a1;
    u32 temp_v1;
    f363_Record *temp_v0;

    temp_a1 = &base[arg0->unk3A];
    temp_v1 = temp_a1->unk178;
    if ((temp_v1 >> 1) & 1)
    {
        temp_a1->unk178 = temp_v1 & ~2;
        temp_v0 = &base[base[arg0->unk3A].unk170];
        temp_v0->unkC = temp_v0->unkC & ~0x2000;
    }
}

/**
 * @brief Clear pending draw bits for every slot linked to an actor.
 * @param rec Caller struct holding the actor slot index at @c unk3A.
 */
void func_8008BCF8(f297_Rec *rec)
{
    s32 i;

    for (i = 0; i < 13; i++)
    {
        if ((D_80105AE0[i].unk178 >> 1) & 1)
        {
            if (D_80105AE0[i].unk170 == rec->unk3A)
            {
                D_80105AE0[i].unk178 = D_80105AE0[i].unk178 & ~2;
                D_80105AE0[rec->unk3A].unkC &= ~0x2000;
            }
        }
    }
}

/**
 * @brief Find the actor whose slot matches a key and reset it.
 * @param key Animation key searched for across the actor-slot table.
 * @return 0 when a matching actor is reset, -1 when none is found.
 */
s32 func_8008BD88(s32 key)
{
    fBD88_Struct_D800FDF58 *scan;
    fBD88_Struct_D800FDF58 *found;
    fBD88_Struct_D80105AE0 *e;
    s32 i;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
        goto found_label;
    e++;
    scan++;
    if (i < 13)
        goto loop;
    found = (fBD88_Struct_D800FDF58 *)-1;
check:
    if (found == (fBD88_Struct_D800FDF58 *)-1)
        return -1;

    found->unk28 = 0xFF;
    found->unk10 = 0;
    found->unk2C++;
    func_800952DC(found, 0);
    field_stop_actor_animations_for_object(found, 1);
    func_80084424(found->unk3A);
    return 0;

found_label:
    found = scan;
    goto check;
}

/**
 * @brief Reset an actor record and reinitialize its associated animation state.
 * @param record Actor record to reset.
 * @param clear_slot Nonzero to clear the associated D_800FD818 entry when its index is below three.
 */
void func_8008BE38(fBE38_FieldActorRecord *record, s32 clear_slot)
{
    fBE38_FieldAnimationRecord *animation_base;
    fBE38_FieldAnimationRecord *animation;
    s32 animation_mask;

    func_8008BC5C(record);
    func_8008BCF8(record);
    record->unk2A = 0x8E;
    if (clear_slot != 0 && record->unk3A < 3)
    {
        D_800FD818[record->unk3A].unk260 = 0;
    }

    record->unk28 = 0xFF;
    animation_base = D_80105AE0;
    record->unk2E = 1;
    record->unk24 = 1;
    record->unk4 = 0;
    record->unk27 = 0;
    record->unk21 = (record->unk21 & 0x80) + 0x1D;

    animation = &animation_base[record->unk3A];
    animation_mask = -0x1801;
    animation->unk174 &= animation_mask;
    field_restart_actor_animation(record);

    field_stop_actor_animations_for_object(record, 1);
    func_80083BC0(record, &D_800FB3C8[record->unk3A * 0x244], 1);
    record->unk1C |= 0x800;
    func_800952DC(record, 0);
    func_80084424(record->unk3A);
}

/**
 * @brief Prime an actor's state fields and clear its slot animation bits.
 * @param arg0 Actor state struct.
 * @param arg1 Value stored into @c unk1B.
 * @param arg2 Low-bit contribution to @c unk21.
 * @param arg3 Value stored into @c unk20.
 */
void func_8008BF88(f364_ArgStruct *arg0, s8 arg1, s32 arg2, s8 arg3)
{
    f364_Record *base = D_80105AE0;
    f364_Record *temp_v0;

    arg0->unk2A = 0x8F;
    arg0->unk2E = 1;
    arg0->unk24 = 1;
    arg0->unk1B = arg1;
    arg0->unk27 = 0;
    arg0->unk21 = (arg0->unk21 & 0x80) + arg2;
    temp_v0 = &base[arg0->unk3A];
    temp_v0->unk174 = temp_v0->unk174 & ~0x1800;
    field_restart_actor_animation(arg0);
    arg0->unk20 = arg3;
}

/**
 * @brief Reset the field state associated with an entry and re-arm its slot.
 * @param arg0 Entry whose associated table records are updated.
 * @param arg1 Value stored into the slot's unk16C field.
 */
void func_8008C024(fC024_EntryA0 *arg0, s8 arg1)
{
    if (arg0->unk3A < 3)
    {
        D_800FD818[arg0->unk3A].unk260 = 0;
    }

    D_80105AE0[arg0->unk3A].unkC &= 0x200;
    D_80105AE0[arg0->unk3A].unk178 |= 0x20;
    D_80105AE0[arg0->unk3A].unk16C = arg1;
    arg0->unk2A = 0xAE;
    arg0->unk20 = 0xA;
}

/**
 * @brief Reset actor state and select the appropriate transition animation.
 * @param actor Actor whose state and animation resources are updated.
 * @return Unspecified value; callers do not consume the result.
 */
s32 func_8008C104(fC104_Actor *actor)
{
    fC104_Record *records, *record, *final_records;
    fC104_Resource *resource, *resources;
    s32 selector;
    func_8008BC5C();
    func_8008BCF8(actor);
    actor->animation = 0x8E;
    actor->byte28 = 0xFF;
    actor->value = 1;
    actor->active = 1;
    actor->word4 = 0;
    actor->byte27 = 0;
    actor->state = (actor->state & 0x80) + 0x1D;
    records = D_80105AE0;
    record = &records[actor->selector];
    record->flags174 &= ~0x1800;
    field_restart_actor_animation(actor);
    field_stop_actor_animations_for_object(actor, 1);
    func_80083BC0(actor, D_800FB3C8 + actor->selector * 0x244, 1);
    actor->flags |= 0x800;
    func_800952DC(actor, 0);
    func_80084424(actor->selector);
    selector = actor->selector;
    if (!(records[selector].flags178 & 1))
    {
        resources = g_field_resource_entries;
        resource = &resources[actor->resource];
        if (resource->flags & 0x8000)
        {
            actor->animation = 0x92;
        }
        else
        {
            func_80083EEC(selector, selector + 0x40, resource->flags);
            field_start_actor_animation(actor->selector + 0x40, 0, 0);
        }
    }
    final_records = D_80105AE0;
    if (!(final_records[actor->selector].flags & 0x200))
    {
        if (actor->animation == 0x92)
        {
            actor->animation = 0x93;
        }
        else
        {
            actor->animation = 0x90;
        }
    }
}

/**
 * @brief Check eligibility and depth overlap for two actor identifiers.
 * @param first_id Identifier whose position supplies the center of the depth test.
 * @param second_id Identifier whose eligibility and position are checked.
 * @return -1 for a missing actor, 1 for an eligible overlap, or 0 otherwise.
 */
s32 func_8008C2EC(s32 first_id, s32 second_id)
{
    fC2EC_Slot *second_slot_scan;
    fC2EC_Slot *first_slot_scan;
    fC2EC_Actor *second_scan;
    fC2EC_Actor *first_scan;
    fC2EC_Actor *second;
    fC2EC_Actor *first;
    s32 second_z;
    s32 first_z;
    s32 first_extent;
    s16 second_extent;
    s32 result;
    s32 index;
    u8 second_slot;
    u8 first_index;
    u8 second_index;
    fC2EC_Slot *slot;

    first_scan = D_800FDF58;
    first_slot_scan = D_80105AE0;
    index = 0;
loop_1:
    index++;
    if (first_slot_scan->id == first_id)
    {
        goto found_first;
    }
    first_slot_scan++;
    first_scan++;
    if (index < 13)
    {
        goto loop_1;
    }
    first = (fC2EC_Actor *)-1;
check_first:
    if (first != (fC2EC_Actor *)-1)
    {
        goto search_second;
    }
    return -1;
found_first:
    first = first_scan;
    goto check_first;
found_second:
    second = second_scan;
    goto check_second;
search_second:
    index = 0;
    second_scan = D_800FDF58;
    second_slot_scan = D_80105AE0;
loop_9:
    index++;
    if (second_slot_scan->id == second_id)
    {
        goto found_second;
    }
    second_slot_scan++;
    second_scan++;
    if (index < 13)
    {
        goto loop_9;
    }
    second = (fC2EC_Actor *)-1;
check_second:
    if (second == (fC2EC_Actor *)-1)
    {
        return -1;
    }
    second_slot = second->slot_index;
    slot = &D_80105AE0[second_slot];
    if (second->presence == 0xFF)
    {
        return -1;
    }
    if (second_slot >= 3U && (slot->group_flags & 15) != D_800FE754)
    {
        return 0;
    }
    if (slot->active == 0)
    {
        return 0;
    }
    second_index = second->slot_index;
    first_index = first->slot_index;
    if (second_index == first_index)
    {
        return 0;
    }
    if (D_8010D020 == 0)
    {
        if (first_index < 3U)
        {
            if (second_index < 3U)
            {
                return 0;
            }
        }
        else if (second_index >= 3U)
        {
            return 0;
        }
    }
    first_z = first->z;
    second_z = second->z;
    first_extent = ((s32)(D_80105AE0[first->slot_index].extent << 16) >> 17) << 8;
    second_extent = (s16)slot->extent;
    if (second_z < first_z - first_extent - (second_extent << 7))
    {
        goto no_overlap;
    }
    result = first_z + first_extent + (second_extent << 7) >= second_z;
    goto done;
no_overlap:
    result = 0;
done:
    return result;
}

/**
 * @brief Advance an actor slot retry counter and restart its associated animation when its retry threshold is reached.
 * @param slot_index Actor slot index to update.
 */
void func_8008C4A8(s32 slot_index)
{
    fC4A8_FieldSlotState *slots;
    fC4A8_FieldSlotState *slot;
    s32 retry_count;
    s32 key;
    fC4A8_FieldSlotState *scan_slot;
    fC4A8_FieldActorState *scan_actor;
    fC4A8_FieldActorState *found_actor;
    s32 i;
    s32 animation_slot;

    slots = D_80105AE0;
    slot = &slots[slot_index];
    if (slot->unkC & 0x8000)
    {
        retry_count = slot->unk18D + 1;
        slot->unk18D = retry_count;
        if ((slot_index < 2) && ((u32)(retry_count & 0xFF) >= (u8)D_800EB068[D_800FD818[slot_index].unk1]))
        {
            key = slot->unk14;
            scan_actor = D_800FDF58;
            scan_slot = slots;
            for (i = 0; i < 0xD; i++, scan_slot++, scan_actor++)
            {
                if (scan_slot->unk14 == key)
                {
                    found_actor = scan_actor;
                    goto scan_done;
                }
            }
            found_actor = (fC4A8_FieldActorState *)-1;
        scan_done:
            if (found_actor != (fC4A8_FieldActorState *)-1)
            {
                animation_slot = func_800839F8(0, 0);
                if ((animation_slot != -1) && (func_80083EEC(found_actor->unk3A, animation_slot, 0x21) != 0))
                {
                    field_start_actor_animation(animation_slot, 0, 0);
                }
            }
            func_8008C620(&D_800FDF58[slot_index]);
        }
    }
}

/**
 * @brief Reset actor control fields and slot flags, then refresh its resources.
 * @param rec Actor state to reset.
 */
void func_8008C620(fC620_FieldActorState *rec)
{
    D_80105AE0[rec->unk3A].unk18D = 0;
    D_80105AE0[rec->unk3A].unkC &= ~0x8000;
    D_80105AE0[rec->unk3A].unkC &= ~0x4000;
    rec->unk2A = 0xB7;
    rec->unk20 = 0x1E;
    rec->unk2E = 1;
    rec->unk30 = 0;
    rec->unk4 = 0;
    rec->unk27 = 0;
    rec->unk21 = (rec->unk21 & 0x80) + 0x13;
    rec->unk24 = 1;
    D_80105AE0[rec->unk3A].unk174 &= ~0x1800;
    field_restart_actor_animation_reverse(rec);
}
