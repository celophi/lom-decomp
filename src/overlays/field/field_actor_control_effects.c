#include "common.h"
#include "sdk/libgpu.h"

/**
 * @file field_actor_control_effects.c
 * @brief Consolidated FIELD actor control, animation-flag, scaling and
 *        ground-shadow/effect-primitive translation unit (vram
 *        0x80086494 .. 0x80087614).
 *
 * Merges the former per-function files func_80086494.c, field337.c,
 * field_set_actor_horizontal_scale.c, field_handle_actor_control_flag_40.c,
 * field_actor_flag_ops.c, func_80086FB8.c and field29.c into a single TU.
 *
 * @note D_80105AE0, D_800FE3A0, D_80107800 and D_801058E0 are each viewed as a
 *       different record type (or element width) by different members, so their
 *       extern declarations are kept at BLOCK scope inside each user with that
 *       user's original type. bcopy has two different prototypes across members
 *       and is likewise declared per-function. Callees left implicitly declared
 *       in their original files (func_80083EEC, field_start_actor_animation,
 *       func_8006C3FC, func_80086C00 in field337 / func_80086494) are kept
 *       implicit here to preserve the original codegen.
 */

/* ---- field29.c types ------------------------------------------------------ */

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padC[0x37 - 0xC];
    u8 unk37;
    u8 pad38[0x3A - 0x38];
    u8 unk3A;
} Rec871A0;

typedef struct {
    u32 unk0;
    u32 unk4;
    s16 unk8;
    s16 unkA;
    u8 unkC;
    u8 unkD;
    u16 unkE;
    s16 unk10;
    s16 unk12;
    u8 unk14;
    u8 unk15;
    u16 unk16;
    s16 unk18;
    s16 unk1A;
    u8 unk1C;
    u8 unk1D;
    u16 unk1E;
    s16 unk20;
    s16 unk22;
    u8 unk24;
    u8 unk25;
    u16 unk26;
} Prim871A0;

typedef struct {
    u16 unk0;
    u16 unk2;
    u16 unk4;
} Off871A0;

typedef struct {
    u16 unk0;
    u16 unk2;
} Screen871A0;

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Scratch871A0;

typedef struct {
    u8 pad0[0x176];
    s16 unk176;
    u8 pad178[0x23C - 0x178];
} State871A0;

typedef struct {
    u8 *start;
    u8 *end;
    u8 unk8;
    u8 slot_index;
    u8 padA[4];
    s16 unkE;
    u32 flags;
} Res871A0;

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
} Rec87564;

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} State87564;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[0x2A - 0x12];
    s16 unk2A;
} Rec875C4;

/* ---- field337.c types ----------------------------------------------------- */

typedef struct
{
    u8 pad0[0x21];
    u8 unk21;   /* 0x21 */
    u8 pad22[0x24 - 0x22];
    u8 unk24;   /* 0x24 */
    u8 pad25[0x27 - 0x25];
    u8 unk27;   /* 0x27 */
    u8 pad28[0x2E - 0x28];
    s16 unk2E;  /* 0x2E */
    u8 pad30[0x3A - 0x30];
    u8 unk3A;   /* 0x3A */
} Actor;

typedef struct
{
    u8 pad0[0x174];
    s32 unk174; /* 0x174 */
    u8 pad178[0x23C - 0x178];
} ActorRec;

/* ---- field_actor_flag_ops.c types ----------------------------------------- */

/**
 * @brief Field actor state record. Only the fields read here are known; the
 *        object index at 0x3A selects the actor's D_80105AE0 slot.
 */
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
    u8 pad20[1];
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
} FieldActorState;

/**
 * @brief Per-actor animation/geometry slot in D_80105AE0; stride 0x23C.
 */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;     /* 0xC flags; bit 0x2000 cleared by the linked-actor helpers */
    u8 pad10[0x170 - 0x10];
    u8 unk170;    /* 0x170 index of the linked actor */
    u8 pad171[0x174 - 0x171];
    u32 unk174;   /* 0x174 */
    u32 unk178;   /* 0x178 bit 1 marks a link to unk170 */
    u8 pad17C[0x23C - 0x17C];
} ActorSlotData;

/** @brief Entry of g_field_actor_slots; stride 0x244. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;   /* 0x24 */
    u8 pad25[0x23A - 0x25];
    u8 unk23A;  /* 0x23A */
    u8 pad23B[0x244 - 0x23B];
} ActorSlot;

/** @brief 0x28-byte record of the D_80107800 table; unk4 is the in-use flag. */
typedef struct
{
    u8 unk0[4];
    u8 unk4;
    u8 unk5[0x23];
} FieldUnkRecord_80086F20;

/* ---- field_handle_actor_control_flag_40.c types --------------------------- */

#define FIELD_ANIMATION_SEQUENCE_MASK 0x7F
#define FIELD_ANIMATION_MIRROR_FLAG 0x80
#define FIELD_ANIMATION_HOLD_LAST_FRAME 0x800

/** @brief Field object state used by the actor-control animation handlers. */
typedef struct
{
    u8 pad00[0x1C];
    s32 state_flags;
    u8 pad20[1];
    u8 animation_sequence;
    u8 pad22[0x24 - 0x22];
    u8 frame_duration_scale;
    u8 pad25[0x27 - 0x25];
    u8 frame_index;
    u8 pad28[0x2E - 0x28];
    u16 animation_repeat_count;
    u8 pad30[0x3A - 0x30];
    u8 object_index;
    u8 pad3B[0x54 - 0x3B];
} FieldObjectRecord;

/** @brief Per-actor runtime slot in D_80105AE0. */
typedef struct
{
    u8 pad000[0x174];
    u32 state;
    u8 pad178[0x23C - 0x178];
} FieldActorSlot;

/* ---- field_set_actor_horizontal_scale.c types ----------------------------- */

typedef struct
{
    u8 pad0[0x3A];
    u8 object_index;
    u8 pad3B[0x54 - 0x3B];
} FieldObjectState;

typedef struct
{
    u8 pad0[0x2E];
    u8 scale_z;
    u8 pad2F[0x33 - 0x2F];
    u8 scale_x;
    u8 pad34[0x48 - 0x34];
} FieldActorPartDef;

/* ---- func_80086494.c types ------------------------------------------------ */

/** @brief Color and animation selector bytes in a 0x54-byte object record. */
typedef struct
{
    u8 pad0[0x18];
    u8 unk18, unk19, unk1a;
    u8 pad1b[10];
    u8 unk25;
    u8 pad26[0x14];
    u8 unk3a;
    u8 tail[0x19];
} FieldControlRecord;
/** @brief Current and previous control flags in a 0x23C-byte runtime state. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkc;
    u8 pad10[0x168];
    union
    {
        s32 word;
        struct
        {
            u8 low[2];
            u8 owner;
            u8 high;
        } bytes;
    } status;
    s32 unk17c;
    u8 tail[0xBC];
} FieldControlState;
/** @brief Animation activity and kind in a 0x244-byte animation actor. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x201];
    u16 unk226;
    u8 pad228[0x12];
    u8 unk23a;
    u8 tail[9];
} FieldControlActor;
/** @brief Color bytes in a 0x48-byte visual record. */
typedef struct
{
    u8 pad0[0xE];
    u8 unke, unkf, unk10;
    u8 tail[0x37];
} FieldControlVisual;

/* ---- func_80086FB8.c types ------------------------------------------------ */

/** @brief Screen translation applied to active effect primitives. */
typedef struct
{
    u16 x;
    u16 y;
} FieldEffectMotion;

/* ---- non-conflicting file-scope extern data ------------------------------- */

extern u32 D_800EB00C[];
extern FieldControlActor D_800FB3C8[];
extern FieldControlRecord D_800FDF58[];
extern ActorSlot g_field_actor_slots[];
extern FieldEffectMotion D_801077FC;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern Res871A0 g_field_resource_entries[];
extern Rec87564 *D_8010A01C;
extern s32 D_8010A030;

/**
 * @brief Dispatch one changed actor control flag and refresh its displayed colors.
 * @param index Object, runtime-state and animation-actor slot index.
 * @note Action values below 0x100 name animations; larger values are callbacks.
 * @note Separate flag masks and explicit table bases preserve target code generation.
 * @note WIP: approximately 94.16% GCC 2.7.2 CDK, with allocation and scheduling residue.
 */
void func_80086494(s32 index)
{
    extern FieldControlState D_80105AE0[];
    extern FieldControlVisual D_800FE3A0[];
    s32 flags_before;
    s32 changed_or_current;
    s32 previous_flags;
    s32 record_offset;
    s32 animation_slot;
    u32 changed_flags;
    s32 flags_current;
    s32 old_flags;
    s32 bit_index;
    s32 clear_mask;
    s32 animation_bit;
    s32 highest_bit;
    s32 bit_mask;
    s32 stop_mask;
    u16 animation_kind;
    u32 *action;
    u32 action_value;
    FieldControlActor *actor;
    FieldControlRecord *record;
    FieldControlState *runtime;
    FieldControlRecord *reset_record;
    FieldControlVisual *visual;
    FieldControlRecord *color_record;

    record_offset = index * 0x54;
    record = (FieldControlRecord *)(record_offset + (u8 *)D_800FDF58);
    runtime = &D_80105AE0[index];
    flags_before = runtime->unkc;
    actor = &D_800FB3C8[index];
    if (flags_before & 0x23E4)
    {
        runtime->unkc &= ~0x4000;
        runtime->unkc &= 0xFFFF7FFF;
    }
    flags_current = runtime->unkc;
    previous_flags = runtime->unk17c;
    changed_or_current = (flags_current ^ previous_flags) | flags_current;
    if (changed_or_current != 0)
    {
        if (actor->unk24 == 0)
        {
            bit_mask = 0x8000;
            bit_index = 0xF;
            animation_slot = index + 0x40;
            action = D_800EB00C;
            action += bit_index;
        find_action:
            if ((changed_or_current & bit_mask) && (action_value = *action, (action_value != 0xFF)))
            {
                if (action_value < 0x100U)
                {
                    if (runtime->unkc & bit_mask)
                    {
                        func_80083EEC(index, animation_slot, action_value);
                        field_start_actor_animation(animation_slot, 0, 0);
                        clear_mask = ~bit_mask;
                    }
                    else
                    {
                        goto update_flags;
                    }
                }
                else
                {
                    ((void (*)(FieldControlRecord *, s32))action_value)(
                        (FieldControlRecord *)(record_offset + (u8 *)D_800FDF58), runtime->unkc & bit_mask);
                update_flags:
                    clear_mask = ~bit_mask;
                }
                runtime->unk17c = (s32)((runtime->unk17c & clear_mask) | (runtime->unkc & bit_mask));
            }
            else
            {
                action--;
                bit_index -= 1;
                bit_mask = bit_mask >> 1;
                if (bit_index < 0)
                {
                }
                else
                {
                    goto find_action;
                }
            }
        }
        else
        {
            if (!(flags_current & 0x8000) && (previous_flags & 0x8000))
            {
                actor->unk24 = 0U;
                actor->unk23a = 0;
                func_8006D21C(actor);
                stop_mask = 0xFFFF7FFF;
                goto clear_stopped;
            }
            if (!(runtime->unkc & 0x4000) && (runtime->unk17c & 0x4000))
            {
                actor->unk24 = 0U;
                actor->unk23a = 0;
                func_8006D21C(actor);
                stop_mask = -0x4001;
            clear_stopped:
                runtime->unk17c = (s32)(runtime->unk17c & stop_mask);
            }
            else
            {
                old_flags = runtime->unk17c;
                changed_flags = (runtime->unkc ^ old_flags) & old_flags;
                if (changed_flags != 0)
                {
                    highest_bit = 0xF;
                find_highest:
                    if ((changed_flags & (1 << highest_bit)) == 0)
                    {
                        highest_bit -= 1;
                        goto find_highest;
                    }
                    animation_kind = actor->unk226;
                    animation_bit = 0x20;
                    switch (animation_kind)
                    {
                    case 5:
                        animation_bit = 0;
                        break;
                    case 6:
                        animation_bit = 1;
                        break;
                    case 7:
                        animation_bit = 2;
                        break;
                    case 8:
                        animation_bit = 3;
                        break;
                    case 11:
                        animation_bit = 4;
                        break;
                    case 10:
                        animation_bit = 5;
                        break;
                    case 9:
                        animation_bit = 6;
                        break;
                    case 12:
                        animation_bit = 7;
                        break;
                    }
                    if (highest_bit == animation_bit)
                    {
                        actor->unk24 = 0U;
                        actor->unk23a = 0;
                        func_8006D21C(actor);
                        if ((runtime->status.word & 1) && (runtime->status.bytes.owner == (record->unk3a + 0x40)))
                        {
                            record->unk25 = 0;
                            runtime->status.word = (s32)(runtime->status.word & ~1);
                        }
                    }
                }
            }
        }
    }
    if (runtime->unkc & 0x10000000)
    {
        reset_record = D_800FDF58;
        reset_record += index;
        reset_record->unk1a = 0x20;
        reset_record->unk19 = 0x20;
        runtime->unkc = (s32)(runtime->unkc & 0xEFFFFFFF);
        return;
    }
    visual = D_800FE3A0;
    visual += index;
    color_record = D_800FDF58;
    color_record += index;
    color_record->unk18 = (u8)visual->unke;
    color_record->unk19 = (u8)visual->unkf;
    color_record->unk1a = (u8)visual->unk10;
}

/**
 * @brief Enter an actor control state and play animation 7 on its +0x40 object.
 * @param arg0 Actor object record.
 * @param arg1 Nonzero to run the transition; zero does nothing.
 * @see decomp.me (100%) TODO
 */
void func_80086850(Actor *arg0, s32 arg1)
{
    extern ActorRec D_80105AE0[];

    if (arg1 != 0)
    {
        arg0->unk2E = 1;
        arg0->unk27 = 0;
        arg0->unk21 &= 0x80;
        arg0->unk24 = 1;
        D_80105AE0[arg0->unk3A].unk174 &= ~0x1800;
        func_8006C3FC(arg0);
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0x7);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        func_80086C00(arg0->unk3A);
    }
}

/**
 * @brief Enter an actor control state and play animation 0xA on its +0x40 object.
 * @param arg0 Actor object record.
 * @param arg1 Nonzero to run the transition; zero does nothing.
 * @see decomp.me (100%) TODO
 */
void func_800868FC(Actor *arg0, s32 arg1)
{
    extern ActorRec D_80105AE0[];

    if (arg1 != 0)
    {
        arg0->unk2E = 1;
        arg0->unk27 = 0;
        arg0->unk21 &= 0x80;
        arg0->unk24 = 1;
        D_80105AE0[arg0->unk3A].unk174 &= ~0x1800;
        func_8006C3FC(arg0);
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xA);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
}

/**
 * @brief Set a field actor's horizontal model scale to half-size or full-size.
 * @param object Field object whose actor-part definition is updated.
 * @param half_scale Non-zero for half-size X/Z scale, zero for full-size scale.
 */
void field_set_actor_horizontal_scale(FieldObjectState *object, s32 half_scale)
{
    extern FieldActorPartDef D_800FE3A0[];

    if (half_scale != 0)
    {
        D_800FE3A0[object->object_index].scale_z = 0x20;
        D_800FE3A0[object->object_index].scale_x = 0x20;
    }
    else
    {
        D_800FE3A0[object->object_index].scale_z = 0x40;
        D_800FE3A0[object->object_index].scale_x = 0x40;
    }
}

/**
 * @brief Handle actor control flag 0x40 becoming active for a field object.
 * @param object Field object whose actor slot and animation state are updated.
 * @param is_set Non-zero when actor control flag 0x40 is currently set.
 */
void func_800869FC(FieldObjectRecord* object, s32 is_set)
{
    s32 func_80083EEC(u8 object_index, s32 actor_index, s32 animation_id);
    void field_start_actor_animation(s32 actor_index, s32 arg1, s32 arg2);
    void func_8006C3FC(FieldObjectRecord* object);
    void func_80086C00(u8 object_index);
    extern FieldActorSlot D_80105AE0[];
    u8 animation_sequence;

    if (is_set != 0)
    {
        func_80083EEC(object->object_index, object->object_index + 0x40, 9);
        field_start_actor_animation(object->object_index + 0x40, 0, 0);
        animation_sequence = object->animation_sequence;
        if ((animation_sequence & FIELD_ANIMATION_SEQUENCE_MASK) != 0x1B)
        {
            object->animation_sequence = (animation_sequence & FIELD_ANIMATION_MIRROR_FLAG) + 0x1B;
            object->animation_repeat_count = 1;
            object->frame_index = 0;
            object->frame_duration_scale = 1;
            D_80105AE0[object->object_index].state &= ~0x1800;
            func_8006C3FC(object);
            object->state_flags |= FIELD_ANIMATION_HOLD_LAST_FRAME;
        }
        func_80086C00(object->object_index);
    }
}

/**
 * @brief Enter or leave the actor's 0x14 control state.
 *
 * With @p flag set, puts the actor into control state 0x14, keeps its slot
 * flags 0x1800 clear, sets bits 0x40800 of unk1C, and runs func_80086C00 on
 * it. With @p flag clear, resets the control state, plays animation 0x91 on
 * the actor's +0x40 object, and clears bit 0x40000 of unk1C.
 *
 * @param rec Actor state record.
 * @param flag Nonzero to enter the state, zero to leave it.
 */
void func_80086ACC(FieldActorState *rec, s32 flag)
{
    void func_8006C3FC(FieldActorState *);
    s32 func_80083EEC(u8, s32, s32);
    void field_start_actor_animation(s32, s32, s32);
    void func_80086C00(s32 idx);
    extern ActorSlotData D_80105AE0[];

    if (flag != 0)
    {
        rec->unk2E = 1;
        rec->unk24 = 1;
        rec->unk27 = 0;
        rec->unk21 = (rec->unk21 & 0x80) + 0x14;
        D_80105AE0[rec->unk3A].unk174 &= ~0x1800;
        func_8006C3FC(rec);
        rec->unk1C |= 0x40800;
        func_80086C00(rec->unk3A);
    }
    else
    {
        rec->unk27 = 0;
        rec->unk2E = 1;
        rec->unk24 = 1;
        rec->unk21 &= 0x80;
        D_80105AE0[rec->unk3A].unk174 &= ~0x1800;
        func_8006C3FC(rec);
        func_80083EEC(rec->unk3A, rec->unk3A + 0x40, 0x91);
        field_start_actor_animation(rec->unk3A + 0x40, 0, 0);
        rec->unk1C &= 0xFFFBFFFF;
    }
}

/**
 * @brief Clears a linked actor's 0x2000 flag when this actor's bit 1 is set.
 *
 * For actor @p idx, if bit 1 of its 0x178 word is set, clears bit 13 (0x2000)
 * of the 0xC flags word belonging to the actor referenced by its 0x170 byte.
 *
 * @param idx Actor slot index.
 */
void func_80086C00(s32 idx)
{
    extern ActorSlotData D_80105AE0[];
    ActorSlotData *base = D_80105AE0;
    ActorSlotData *e = &base[idx];

    if ((e->unk178 >> 1) & 1)
    {
        ActorSlotData *e2 = &base[e->unk170];
        e2->unkC &= ~0x2000;
    }
}

/**
 * @brief Play animation 0xC or 0xD on the actor's +0x40 object and set unk25.
 *
 * With @p arg1 set, plays 0xC, marks unk25 = 0xFE, and clears the linked
 * actor's 0x2000 flag as func_80086C00 does. With @p arg1 clear, plays 0xD
 * and zeroes unk25.
 *
 * @param arg0 Actor state record.
 * @param arg1 Selects the 0xC (nonzero) or 0xD (zero) path.
 */
void func_80086C70(FieldActorState *arg0, s32 arg1)
{
    s32 func_80083EEC(u8, s32, s32);
    void field_start_actor_animation(s32, s32, s32);
    extern ActorSlotData D_80105AE0[];

    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xC);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        arg0->unk25 = 0xFE;
        {
            ActorSlotData *base = D_80105AE0;
            ActorSlotData *e = &base[arg0->unk3A];
            if ((e->unk178 >> 1) & 1)
            {
                ActorSlotData *e2 = &base[e->unk170];
                e2->unkC &= ~0x2000;
            }
        }
    }
    else
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xD);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        arg0->unk25 = 0;
    }
}

/**
 * @brief Clears a linked actor's 0x2000 flag from a record's actor index.
 *
 * Uses the 0x3A index byte of @p p to select an actor; if bit 1 of its 0x178
 * word is set, clears bit 13 (0x2000) of the 0xC flags word belonging to the
 * actor referenced by its 0x170 byte.
 *
 * @param p Actor state record viewed as bytes.
 */
void func_80086D5C(u8 *p)
{
    extern ActorSlotData D_80105AE0[];
    ActorSlotData *base = D_80105AE0;
    ActorSlotData *e = &base[p[0x3A]];

    if ((e->unk178 >> 1) & 1)
    {
        ActorSlotData *e2 = &base[e->unk170];
        e2->unkC &= ~0x2000;
    }
}

/**
 * @brief Play animation 0xE on the actor's +0x40 object, or clear its slot bytes 0x24 and 0x23A.
 * @param arg0 Actor state record.
 * @param arg1 Nonzero plays the animation, zero clears the slot bytes.
 * @see decomp.me (100%) TODO
 */
void func_80086DD0(FieldActorState *arg0, s32 arg1)
{
    s32 func_80083EEC(u8, s32, s32);
    void field_start_actor_animation(s32, s32, s32);

    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xE);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
    else
    {
        g_field_actor_slots[arg0->unk3A + 0x40].unk24 = 0;
        g_field_actor_slots[arg0->unk3A + 0x40].unk23A = 0;
    }
}

/**
 * @brief Play animation 0x19 on the actor's +0x40 object, or clear its slot bytes 0x24 and 0x23A.
 * @param arg0 Actor state record.
 * @param arg1 Nonzero plays the animation, zero clears the slot bytes.
 * @see decomp.me (100%) TODO
 */
void func_80086E78(FieldActorState *arg0, s32 arg1)
{
    s32 func_80083EEC(u8, s32, s32);
    void field_start_actor_animation(s32, s32, s32);

    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0x19);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
    else
    {
        g_field_actor_slots[arg0->unk3A + 0x40].unk24 = 0;
        g_field_actor_slots[arg0->unk3A + 0x40].unk23A = 0;
    }
}

/**
 * @brief Clear the unk4 flag byte across all 256 records of the D_80107800
 *        table.
 */
void func_80086F20(void)
{
    extern FieldUnkRecord_80086F20 D_80107800[];
    s32 i;

    for (i = 0xFF; i >= 0; i--)
    {
        D_80107800[i].unk4 = 0;
    }
}

/**
 * @brief Stores a record into the first free slot of the D_80107800 table.
 *
 * Scans up to 256 records for one whose unk4 flag byte is clear, copies 0x28
 * bytes from @p src into it, and writes @p value to the parallel D_801058E0
 * half-word slot.
 *
 * @param src Source record, 0x28 bytes.
 * @param value Halfword stored in the parallel D_801058E0 slot.
 */
void func_80086F48(const void *src, s16 value)
{
    void *bcopy(const void *, void *, int);
    extern FieldUnkRecord_80086F20 D_80107800[];
    extern s16 D_801058E0[];
    s32 i = 0;
    s16 *slot = D_801058E0;
    FieldUnkRecord_80086F20 *entry = D_80107800;

    for (; i < 0x100; i++)
    {
        if (entry->unk4 == 0)
        {
            bcopy(src, entry, 0x28);
            *slot = value;
            return;
        }
        slot++;
        entry++;
    }
}

/**
 * @brief Fade, translate, and enqueue active primitives from the 256-entry effect pool.
 * @param buffer Ordering table and packet-buffer state, with the write cursor at offset 0x40B8.
 */
void func_80086FB8(u8 *buffer)
{
    extern POLY_FT4 D_80107800[];
    extern u16 D_801058E0[];
    extern void bcopy(void *, void *, s32);
    POLY_FT4 *source;
    POLY_FT4 *output;
    u32 *ordering;
    s32 i;
    u8 color;

    output = *(POLY_FT4 **)(buffer + 0x40B8);
    ordering = (u32 *)buffer;
    source = D_80107800;

    for (i = 0; i < 256;)
    {
        color = source->r0;
        if (color != 0)
        {
            if (color < 0x10)
            {
                color = 0;
            }
            else
            {
                color -= 0x10;
            }

            source->b0 = color;
            source->g0 = color;
            source->r0 = color;
            source->code |= 2;
            source->x0 += D_801077FC.x;
            source->x1 += D_801077FC.x;
            source->x2 += D_801077FC.x;
            source->x3 += D_801077FC.x;
            source->y0 += D_801077FC.y;
            source->y1 += D_801077FC.y;
            source->y2 += D_801077FC.y;
            source->y3 += D_801077FC.y;

            bcopy(source, output, sizeof(POLY_FT4));
            addPrim(&ordering[D_801058E0[i]], output);
            output++;
        }

        i++;
        source++;
    }

    *(POLY_FT4 **)(buffer + 0x40B8) = output;
}

/**
 * @brief Compute an actor's screen-space ground shadow and emit its POLY_FT4
 *        primitive into the ordering table.
 * @param arg0 Effect record supplying world x/z and the shadow selectors at
 *             @c unk37 / @c unk3A.
 * @param arg1 Primitive cursor to fill; returned advanced past the emitted
 *             primitive when one is produced.
 * @param arg2 Ordering-table base array (0x1000 entries) linked into.
 * @param arg3 Shadow footprint half-extents (x at @c unk0, y at @c unk4).
 * @return The (possibly advanced) primitive cursor.
 * @note WIP - 90.73% (101/241 exact rows). Body is raw m2c output kept
 *       verbatim to preserve the verified match; brace style will be
 *       normalised to Allman when the function reaches 100%. Residue is
 *       concentrated in argdiff rows (register coloring) around the two
 *       footprint-projection arms and the OT-link tail.
 * @see decomp.me WIP
 */
Prim871A0 *func_800871A0(Rec871A0 *arg0, Prim871A0 *arg1, s32 *arg2, Off871A0 *arg3)
{
    extern State871A0 D_80105AE0[];
    extern Res871A0 g_field_resource_entries[];
    s32 temp_t3;
    s16 temp_v0_3;
    s16 temp_v0_4;
    s16 temp_v0_5;
    s16 temp_v1;
    s16 var_v0_3;
    s16 var_v0_4;
    s32 *temp_v0_6;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a0_3;
    s32 temp_a2_2;
    s32 temp_t1;
    s32 temp_v0;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s32 var_a0;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    s32 var_v1_2;
    s32 temp_t4;
    u8 temp_a1;
    s32 temp_v0_2;
    s32 cam_x_q;
    s32 rec_x_q;
    s32 cam_y_q;
    s32 rec_z_q;
    s32 screen_y_base;
    s32 work_v1;
    s32 work_a0;
    s32 temp_t7;
    Screen871A0 *screen;
    Scratch871A0 *scratch;
    Prim871A0 *var_t0;
    s32 *var_t6;

    var_t0 = arg1;
    var_t6 = arg2;
    screen = (Screen871A0 *)0x1F8000C0;
    scratch = (Scratch871A0 *)0x1F8000C4;
    var_a1 = D_800F22A0;
    temp_v0 = arg0->unk0;
    scratch->unk4 = 0;
    scratch->unk0 = temp_v0;
    temp_a2_2 = arg0->unk8;
    scratch->unk8 = temp_a2_2;
    if (var_a1 < 0) {
        var_a1 += 0xFF;
    }
    var_a0 = temp_v0;
    cam_x_q = var_a1 >> 8;
    if (var_a0 < 0) {
        var_a0 += 0xFF;
    }
    var_a1_2 = D_800F22A4;
    rec_x_q = var_a0 >> 8;
    rec_x_q += 0xA0;
    temp_t4 = cam_x_q + rec_x_q;
    screen->unk0 = temp_t4;
    if (var_a1_2 < 0) {
        var_a1_2 += 0xFF;
    }
    var_a0 = temp_a2_2;
    cam_y_q = var_a1_2 >> 8;
    screen_y_base = cam_y_q + 0x70;
    if (var_a0 < 0) {
        var_a0 += 0x1FF;
    }
    var_v1 = D_800F22A8;
    rec_z_q = var_a0 >> 9;
    screen_y_base -= rec_z_q;
    if (var_v1 < 0) {
        var_v1 += 0x1FF;
    }
    screen->unk2 = (u16)(screen_y_base - (var_v1 >> 9));
    temp_v0_2 = arg0->unk37;
    temp_a1 = arg0->unk3A;
    temp_t1 = temp_v0_2 << 0x18;
    temp_t3 = D_80105AE0[temp_a1].unk176;
    temp_t7 = temp_t1 >> 24;
    if (g_field_resource_entries[temp_a1].unk8 != 0) {
        temp_a2_2 = temp_t3 << 8;
        var_a1 = temp_t1 >> 0x1A;
        var_v0 = arg0->unk4;
        work_a0 = (u16)arg3->unk0;
        var_v0 -= temp_a2_2;
        var_v0 >>= 0xB;
        var_v0 += var_a1;
        work_v1 = var_v0 << 2;
        var_v0 += work_v1;
        work_v1 = temp_t4 + work_a0;
        if (var_v0 < 0) {
            var_v0 += 3;
        }
        var_v0 >>= 2;
        var_v0 = work_v1 - var_v0;
        var_t0->unk8 = var_v0;
        var_t0->unk18 = var_v0;

        var_v0_2 = arg0->unk4;
        work_v1 = (u16)arg3->unk4;
        var_v0_2 -= temp_a2_2;
        var_v0_2 >>= 0xB;
        var_v0_2 -= var_a1;
        work_a0 = var_v0_2 << 2;
        var_a1 = screen->unk0;
        var_v0_2 += work_a0;
        var_a1 += work_v1;
        if (var_v0_2 < 0) {
            var_v0_2 += 3;
        }
        var_v0_2 >>= 2;
        var_v0_3 = var_a1 + var_v0_2;
    } else {
        temp_a2_2 = temp_t3 << 8;
        var_a1 = temp_t1 >> 0x1A;
        work_v1 = (u16)arg3->unk0;
        var_v0 = arg0->unk4;
        work_v1 += temp_t4;
        var_v0 -= temp_a2_2;
        var_v0 >>= 0xB;
        work_v1 -= var_v0;
        work_v1 += var_a1;
        var_t0->unk8 = work_v1;
        var_t0->unk18 = work_v1;

        var_v0_2 = screen->unk0;
        work_a0 = (u16)arg3->unk4;
        work_v1 = arg0->unk4;
        var_v0_2 += work_a0;
        work_v1 -= temp_a2_2;
        work_v1 >>= 0xB;
        var_v0_2 += work_v1;
        var_v0_3 = var_v0_2 - var_a1;
    }
    var_t0->unk10 = var_v0_3;
    var_t0->unk20 = var_v0_3;
    var_a0 = temp_t3 << 8;
    var_v1 = (s16)arg3->unk0;
    var_v0 = (s16)arg3->unk4;
    var_a1 = var_t0->unk8;
    var_v1 -= var_v0;
    var_v1 >>= 1;
    var_v0 = arg0->unk4;
    if (var_v1 < 0) {
        var_v1 = -var_v1;
    }
    var_v0 -= var_a0;
    var_v0 >>= 0xB;
    var_v1 += var_v0;
    var_a0 = temp_t7 >> 2;
    var_v0 = var_t0->unk10;
    var_v0 = var_v0 < var_a1;
    var_v1 -= var_a0;
    if (var_v0 == 0) {
        var_v0 = var_v1 < 2;
        if (var_v0 == 0) {
            if (temp_t3 != 0) {
                var_v0 = screen->unk2;
                var_v1 >>= 1;
                var_v0 -= var_v1;
                var_v0 += temp_t3;
                var_t0->unk12 = var_v0;
                var_t0->unkA = var_v0;
                var_v0 = screen->unk2;
                var_v0 += var_v1;
                var_v0 += temp_t3;
            } else {
                var_v0 = screen->unk2;
                var_v1 >>= 1;
                var_v0 -= var_v1;
                var_t0->unk12 = var_v0;
                var_t0->unkA = var_v0;
                var_v0 = screen->unk2;
                var_v0 += var_v1;
            }
            var_t0->unk22 = var_v0;
            var_t0->unk1A = var_v0;
            var_t0->unk4 = 0x808080;
            ((u8 *)var_t0)[3] = 9;
            ((u8 *)var_t0)[7] = 0x2E;
            var_t0->unk24 = 0x40;
            var_t0->unk14 = 0x40;
            var_t0->unk15 = 0x50;
            var_t0->unkD = 0x50;
            var_t0->unk25 = 0x70;
            var_t0->unk1D = 0x70;
            var_t0->unk16 = 0x5F;
            var_t0->unk1C = 0;
            var_t0->unkC = 0;
            var_t0->unkE = 0x7850;
            temp_v1_5 = (s32)arg0->unk8 >> 7;
            {
                typedef struct { unsigned addr:24; unsigned len:8; } LocalTag;
                if (temp_v1_5 < 0) {
                    ((LocalTag *)var_t0)->addr = ((LocalTag *)&var_t6[0])->addr;
                    ((LocalTag *)&var_t6[0])->addr = (u32)var_t0;
                    var_t0 = (Prim871A0 *)((u8 *)var_t0 + 0x28);
                } else if (temp_v1_5 >= 0x1000) {
                    ((LocalTag *)var_t0)->addr = ((LocalTag *)&var_t6[0xFFF])->addr;
                    ((LocalTag *)&var_t6[0xFFF])->addr = (u32)var_t0;
                    var_t0 = (Prim871A0 *)((u8 *)var_t0 + 0x28);
                } else {
                    ((LocalTag *)var_t0)->addr = ((LocalTag *)&var_t6[temp_v1_5])->addr;
                    ((LocalTag *)&var_t6[(s32)arg0->unk8 >> 7])->addr = (u32)var_t0;
                    var_t0 = (Prim871A0 *)((u8 *)var_t0 + 0x28);
                }
            }
        }
    }
    return var_t0;
}

/**
 * @brief Hand the actor's animation record to func_800B2198 and cache the record.
 * @param arg0 Actor record whose 0x3A index selects the D_80105AE0 slot.
 */
void func_80087564(Rec87564 *arg0)
{
    extern State87564 D_80105AE0[];

    D_8010A01C = arg0;
    func_800B2198(D_80105AE0[arg0->unk3A].unk14, D_80105AE0);
}

/**
 * @return Value of D_8010A030.
 * @see decomp.me (100%) N/A -- trivial 4-instruction leaf function, no scratch needed.
 */
s32 func_800875B4(void)
{
    return D_8010A030;
}

/**
 * @brief Report whether the current record's 0x10 and 0x2A halfwords are both zero.
 * @return 1 when both are zero, 0 when only 0x2A is nonzero, -1 when the record
 *         lookup fails.
 */
s32 func_800875C4(void)
{
    Rec875C4 *rec = func_80087C9C();
    s32 result;

    if (rec == (Rec875C4 *)-1)
    {
        return -1;
    }

    result = 0;
    if (rec->unk10 == 0)
    {
        result = rec->unk2A == 0;
    }

    return result;
}
