#include "common.h"
#include "gpu_packet.h"
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
 *       field_restart_actor_animation, func_80086C00 in field337 / func_80086494) are kept
 *       implicit here to preserve the original codegen.
 */

/* ---- Ground-shadow renderer types --------------------------------------- */

/** @brief Actor position and shadow parameters used by the ground-shadow renderer. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u8 pad_0x0c[0x37 - 0xC];
    u8 shadow_bias;
    u8 pad_0x38[0x3A - 0x38];
    u8 resource_index;
} ShadowActor;

/** @brief First two footprint corners; only their horizontal coordinates are used. */
typedef struct
{
    s16 left_x;
    s16 first_y;
    s16 right_x;
} ShadowFootprint;

/** @brief Projected ground position in the field renderer scratchpad. */
typedef struct
{
    u16 x;
    u16 y;
} ShadowScreenPosition;

/** @brief World position copied to the field renderer scratchpad. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} ShadowWorldPosition;

/** @brief Ground-shadow height in the 0x23C-byte actor slot. */
typedef struct
{
    u8 pad_0x000[0x176];
    s16 shadow_height;
    u8 pad_0x178[0x23C - 0x178];
} ShadowActorSlot;

/** @brief Resource entry view used to select the shadow inset scale. */
typedef struct
{
    u8 *start;
    u8 *end;
    u8 shadow_scale_mode;
    u8 slot_index;
    u8 pad_0x0a[4];
    s16 unknown_0x0e;
    u32 flags;
} ShadowResourceEntry;

#define SHADOW_SCREEN_POSITION ((ShadowScreenPosition *)0x1F8000C0)
#define SHADOW_WORLD_POSITION ((ShadowWorldPosition *)0x1F8000C4)
#define SHADOW_SCREEN_CENTER_X 160
#define SHADOW_SCREEN_CENTER_Y 112
#define SHADOW_OT_SIZE 4096
#define SHADOW_DEPTH_SHIFT 7

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
extern ShadowResourceEntry g_field_resource_entries[];
extern Rec87564 *D_8010A01C;
extern s32 D_8010A030;

/**
 * @brief Dispatch one changed actor control flag and refresh its displayed colors.
 * @param index Object, runtime-state and animation-actor slot index.
 * @note Action values below 0x100 name animations; larger values are callbacks.
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
    s32 scan_flags;
    s32 bit_index;
    s32 clear_mask;
    s32 animation_bit;
    s32 highest_bit;
    s32 bit_mask;
    u16 animation_kind;
    u32 *action;
    u32 *action_base;
    u32 action_value;
    FieldControlActor *actor;
    FieldControlRecord *record;
    FieldControlState *runtime;

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
            do
            {
                bit_mask = 0x8000;
                bit_index = 0xF;
                animation_slot = index + 0x40;
                action_base = D_800EB00C;
                action = action_base + bit_index;
            find_action:
            if ((changed_or_current & bit_mask) && (action_value = *action, (action_value != 0xFF)))
            {
                if (action_value < 0x100U)
                {
                    if (runtime->unkc & bit_mask)
                    {
                        func_80083EEC(index, animation_slot, action_value);
                        field_start_actor_animation(animation_slot, 0, 0);
                    }
                }
                else
                {
                    ((void (*)(FieldControlRecord *, s32))action_value)(
                        (FieldControlRecord *)(record_offset + (u8 *)D_800FDF58), runtime->unkc & bit_mask);
                }
                clear_mask = ~bit_mask;
                runtime->unk17c = (s32)((runtime->unk17c & clear_mask) | (runtime->unkc & bit_mask));
            }
            else
            {
                action--;
                bit_index--;
                bit_mask >>= 1;
                if (bit_index >= 0)
                {
                    goto find_action;
                }
            }
            } while (0);
        }
        else
        {
            if (!(flags_current & 0x8000) && (previous_flags & 0x8000))
            {
                actor->unk24 = 0U;
                actor->unk23a = 0;
                field_clear_actor_effects(actor);
                runtime->unk17c &= 0xFFFF7FFF;
            }
            else if (!(runtime->unkc & 0x4000) && (runtime->unk17c & 0x4000))
            {
                actor->unk24 = 0U;
                actor->unk23a = 0;
                field_clear_actor_effects(actor);
                runtime->unk17c &= -0x4001;
            }
            else
            {
                changed_flags = runtime->unkc;
                old_flags = runtime->unk17c;
                changed_flags = (changed_flags ^ old_flags) & old_flags;
                if (changed_flags != 0)
                {
                    scan_flags = (s32)changed_flags;
                    highest_bit = 0xF;
                    animation_bit = 1;
                find_highest:
                    if ((scan_flags & (animation_bit << highest_bit)) == 0)
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
                        field_clear_actor_effects(actor);
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
        D_800FDF58[index].unk1a = 0x20;
        D_800FDF58[index].unk19 = 0x20;
        runtime->unkc = (s32)(runtime->unkc & 0xEFFFFFFF);
    }
    else
    {
        D_800FDF58[index].unk18 = D_800FE3A0[index].unke;
        D_800FDF58[index].unk19 = D_800FE3A0[index].unkf;
        D_800FDF58[index].unk1a = D_800FE3A0[index].unk10;
    }
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
        field_restart_actor_animation(arg0);
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
        field_restart_actor_animation(arg0);
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
    void field_restart_actor_animation(FieldObjectRecord* object);
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
            field_restart_actor_animation(object);
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
    void field_restart_actor_animation(FieldActorState *);
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
        field_restart_actor_animation(rec);
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
        field_restart_actor_animation(rec);
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
            setSemiTrans(source, 1);
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
 * @brief Project an actor's ground shadow and append its textured quad.
 * @param actor World position, shadow bias and resource slot.
 * @param primitives Next free POLY_FT4 in the primitive buffer.
 * @param ordering_table Depth ordering table with 4096 entries.
 * @param footprint First two footprint corners supplying the horizontal bounds.
 * @return Next free primitive, unchanged if the shadow has collapsed.
 * @see decomp.me (100%)
 */
POLY_FT4 *field_render_actor_ground_shadow(ShadowActor *actor, POLY_FT4 *primitives, s32 *ordering_table, ShadowFootprint *footprint)
{
    extern ShadowActorSlot D_80105AE0[];
    extern ShadowResourceEntry g_field_resource_entries[];
    s16 edge_y;
    s32 shadow_height;
    s16 right_x;
    s32 ground_y;
    s32 ground_height_fixed;
    s32 world_z;
    s32 bias_bits;
    s32 world_x;
    s32 depth;
    s32 projection_value;
    s32 camera_offset;
    s32 horizontal_offset;
    s32 left_inset;
    s32 right_inset;
    s32 diameter;
    s32 camera_z;
    s32 screen_x;
    u8 resource_index;
    s32 camera_x;
    s32 actor_screen_x;
    s32 camera_y;
    s32 actor_depth_y;
    s32 camera_screen_y;
    s32 edge_work;
    s32 scale_work;
    s32 shadow_bias;
    ShadowScreenPosition *screen;
    ShadowWorldPosition *scratch;

    /* Project the ground point; negative fixed-point values round toward zero. */
    screen = SHADOW_SCREEN_POSITION;
    scratch = SHADOW_WORLD_POSITION;
    camera_offset = D_800F22A0;
    world_x = actor->x;
    scratch->y = 0;
    scratch->x = world_x;
    world_z = actor->z;
    ground_height_fixed = world_z;
    scratch->z = ground_height_fixed;
    if (camera_offset < 0)
    {
        camera_offset += 0xFF;
    }
    projection_value = world_x;
    camera_x = camera_offset >> 8;
    if (projection_value < 0)
    {
        projection_value += 0xFF;
    }
    camera_offset = D_800F22A4;
    actor_screen_x = (projection_value >> 8) + SHADOW_SCREEN_CENTER_X;
    screen_x = camera_x + actor_screen_x;
    screen->x = screen_x;
    if (camera_offset < 0)
    {
        camera_offset += 0xFF;
    }
    projection_value = world_z;
    camera_y = camera_offset >> 8;
    camera_screen_y = camera_y + SHADOW_SCREEN_CENTER_Y;
    if (projection_value < 0)
    {
        projection_value += 0x1FF;
    }
    camera_z = D_800F22A8;
    actor_depth_y = projection_value >> 9;
    ground_y = camera_screen_y - actor_depth_y;
    if (camera_z < 0)
    {
        camera_z += 0x1FF;
    }
    screen->y = (u16)(ground_y - (camera_z >> 9));

    /* The resource profile widens the height-dependent inset by 5/4. */
    resource_index = actor->resource_index;
    bias_bits = actor->shadow_bias << 24;
    shadow_height = D_80105AE0[resource_index].shadow_height;
    shadow_bias = bias_bits >> 24;
    if (g_field_resource_entries[resource_index].shadow_scale_mode != 0)
    {
        ground_height_fixed = shadow_height << 8;
        horizontal_offset = bias_bits >> 26;
        left_inset = actor->y;
        scale_work = (u16)footprint->left_x;
        left_inset = (left_inset - ground_height_fixed) >> 11;
        left_inset += horizontal_offset;
        edge_work = left_inset << 2;
        left_inset = edge_work - -left_inset;
        edge_work = screen_x + scale_work;
        if (left_inset < 0)
        {
            left_inset += 3;
        }
        left_inset >>= 2;
        left_inset = edge_work - left_inset;
        primitives->x0 = left_inset;
        primitives->x2 = left_inset;

        right_inset = actor->y;
        edge_work = (u16)footprint->right_x;
        right_inset = (right_inset - ground_height_fixed) >> 11;
        right_inset -= horizontal_offset;
        scale_work = right_inset << 2;
        horizontal_offset = screen->x;
        right_inset = scale_work - -right_inset;
        horizontal_offset += edge_work;
        if (right_inset < 0)
        {
            right_inset += 3;
        }
        right_inset >>= 2;
        right_x = horizontal_offset + right_inset;
    }
    else
    {
        s32 height_delta;
        s32 ground_height_fixed_local;
        s32 horizontal_offset_local;
        s32 left_inset_local;
        s32 right_inset_local;
        s32 scale_work_local;

        ground_height_fixed_local = shadow_height << 8;
        horizontal_offset_local = bias_bits >> 26;
        edge_work = (u16)footprint->left_x;
        left_inset_local = actor->y;
        edge_work = screen_x - -edge_work;
        left_inset_local = (left_inset_local - ground_height_fixed_local) >> 11;
        edge_work -= left_inset_local;
        edge_work += horizontal_offset_local;
        primitives->x0 = edge_work;
        primitives->x2 = edge_work;

        do
        {
            right_inset_local = screen->x;
        } while (0);
        scale_work_local = (u16)footprint->right_x;
        do
        {
            height_delta = actor->y;
            right_inset_local += scale_work_local;
        } while (0);
        height_delta -= ground_height_fixed_local;
        height_delta >>= 11;
        right_inset_local += height_delta;
        right_x = right_inset_local - horizontal_offset_local;
    }
    do
    {
        primitives->x1 = right_x;
        primitives->x3 = right_x;
    } while (0);

    /* Reject inverted horizontal bounds or a vertical diameter below two pixels. */
    projection_value = shadow_height << 8;
    diameter = (s16)footprint->left_x;
    left_inset = (s16)footprint->right_x;
    camera_offset = primitives->x0;
    diameter -= left_inset;
    diameter >>= 1;
    left_inset = actor->y;
    diameter = abs(diameter);
    left_inset -= projection_value;
    left_inset >>= 11;
    diameter += left_inset;
    projection_value = shadow_bias >> 2;
    diameter -= projection_value;
    if (primitives->x1 >= camera_offset && diameter >= 2)
    {
        if (shadow_height != 0)
        {
            diameter >>= 1;
            edge_y = screen->y - diameter + shadow_height;
            primitives->y1 = edge_y;
            primitives->y0 = edge_y;
            edge_y = screen->y + diameter + shadow_height;
            primitives->y3 = edge_y;
            primitives->y2 = edge_y;
        }
        else
        {
            diameter >>= 1;
            edge_y = screen->y - diameter;
            primitives->y1 = edge_y;
            primitives->y0 = edge_y;
            edge_y = screen->y + diameter;
            primitives->y3 = edge_y;
            primitives->y2 = edge_y;
        }

        /* Neutral modulation with subtractive blending for the shadow texture. */
        SET_BGR0_PACKED(primitives, GPU_TINT_NEUTRAL);
        setPolyFT4(primitives);
        setSemiTrans(primitives, 1);
        primitives->u3 = 0x40;
        primitives->u1 = 0x40;
        primitives->v1 = 0x50;
        primitives->v0 = 0x50;
        primitives->v3 = 0x70;
        primitives->v2 = 0x70;
        setTPage(primitives, 0, 2, 960, 256);
        primitives->u2 = 0;
        primitives->u0 = 0;
        setClut(primitives, 256, 481);

        depth = actor->z >> SHADOW_DEPTH_SHIFT;
        if (depth < 0)
        {
            addPrim(&ordering_table[0], primitives);
            primitives++;
        }
        else if (depth >= SHADOW_OT_SIZE)
        {
            addPrim(&ordering_table[SHADOW_OT_SIZE - 1], primitives);
            primitives++;
        }
        else
        {
            addPrim(&ordering_table[actor->z >> SHADOW_DEPTH_SHIFT], primitives);
            primitives++;
        }
    }
    return primitives;
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
