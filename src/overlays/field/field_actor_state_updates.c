/** @file field_actor_state_updates.c
 * @brief Actor movement/trigger states, pending actions, displacement and follow
 *        movement, resource-state waits, animation resume, and the object
 *        sequence interpreter with its animation actors and tint flashing.
 *
 * One translation unit: the jump tables of func_80092C98 (0x80050F14) and
 * field_execute_actor_sequence (0x8005100C) share one object's .rodata; the
 * zero word at 0x80051008 between them is the compiler's 8-byte alignment
 * before the second table.
 */
#include "field_actor_sequence_runtime.h"

/** @brief Player metadata selecting the bank of actor sequence rows. */
typedef struct
{
    u8 flags;
    u8 sequence_bank;
    u8 pad2[0x266];
} FieldSequencePlayer;

/** @brief Actor template followed by the remaining per-player record data. */
typedef struct
{
    FieldActorState actor;
    u8 tail[0x24];
} FieldSequenceTemplate;

extern FieldActorPartDef g_field_object_parts[];
extern FieldSequencePlayer g_field_player_records[];
extern FieldSequenceTemplate g_field_actor_templates[];
extern FieldActorState g_field_shared_actor_template;
extern u8 g_field_actor_sequence_data[];
extern FieldActorState g_field_actor_slots[];
extern FieldMotionRecord g_field_actors[];
extern s32 g_field_active_group;
extern s32 g_frame_counter;

/* field_actor_movement_states: Dispatch actor movement and trigger states and start checked animations. */

/* field_actor_movement_states */
/* func_80092AD8 */
#include "common.h"

/**
 * @brief Record fields consumed by the FIELD movement-state update.
 */
typedef struct
{
    u8 pad0[4];
    s32 state_value;
    u8 pad8[0x21 - 8];
    u8 state_flags;
    u8 pad22[0x54 - 0x22];
} FieldStateRecord;

void field_restart_actor_animation();
void func_80092C24(u8 *rec, s32 arg1);
s32 field_resolve_actor_movement();

/**
 * @brief Advance selected FIELD movement states and request animation 0x1A.
 * @param entry Record containing the signed state value and state flags.
 * @return Zero while the state is being advanced, or one when it is complete.
 * @note The high state bit selects horizontal displacement direction.
 * @note Local assembly match: 100% with GCC 2.7.2 CDK (83 instructions).
 * @see decomp.me WIP
 */
s32 func_80092AD8(FieldStateRecord *entry)
{
    s32 state_value;
    s32 unused_value;
    s32 timer;
    /**
     * @brief Three-axis displacement stored in scratchpad RAM.
     */
    struct Vector
    {
        s32 x;
        s32 y;
        s32 z;
    } *scratch;

    scratch = (struct Vector *)0x1F800000;
    switch ((u32)(u8)(entry->state_flags & 0x7F) - 8)
    {
        case 0: /* state 8 */
            entry->state_flags = (entry->state_flags & 0x80) | 9;
            field_restart_actor_animation(entry);
            return 0;
        case 53: /* state 61 */
            state_value = entry->state_value;
            if (state_value < -0xC00)
            {
                entry->state_value = state_value + 0xC00;
                return 0;
            }
            else
            {
                entry->state_value = 0;
                entry->state_flags = (entry->state_flags & 0x80) | 9;
                field_restart_actor_animation(entry);
                return 0;
            }
        case 64: /* state 72 */
        case 65: /* state 73 */
            state_value = entry->state_value;
            if (state_value < -0xC00)
            {
                if (entry->state_flags & 0x80)
                {
                    scratch->x = 0x200;
                }
                else
                {
                    scratch->x = -0x200;
                }
                scratch->z = 0;
                scratch->y = 0;
                timer = entry->state_value;
                field_resolve_actor_movement(entry, scratch, 1);
                timer += 0xC00;
                entry->state_value = timer;
                return 0;
            }
            else if (state_value == 0)
            {
                break;
            }
            else if (state_value < -0xA)
            {
                entry->state_value = -0xA;
                func_80092C24((u8 *)entry, 0x1A);
            }
            else
            {
                entry->state_value = state_value + 1;
            }
            return 0;
        case 1:  /* state 9  */
        case 50: /* state 58 */
        case 51: /* state 59 */
        case 52: /* state 60 */
        case 62: /* state 70 */
        case 63: /* state 71 */
        case 70: /* state 78 */
        case 71: /* state 79 */
            func_80092C24((u8 *)entry, 0x1A);
            return 1;
        default:
            break;
    }
    return 1;
}

/* func_80092C24 */
#include "common.h"

extern s32 func_800839F8(s32 arg0, s32 arg1);
s32 func_80083EEC();
void field_start_actor_animation();

/**
 * @brief Starts an actor's animation when its slot resolves and passes a check.
 *
 * Resolves the actor slot for @p rec's 0x3A id via func_800839F8; if valid and
 * func_80083EEC (given @p arg1) succeeds, starts that slot's animation.
 */
void func_80092C24(u8 *rec, s32 arg1)
{
    s32 v = func_800839F8(rec[0x3A], 0);

    if (v != -1)
    {
        if (func_80083EEC(rec[0x3A], v, arg1))
        {
            field_start_actor_animation(v, 0, 0);
        }
    }
}

/* func_80092C98 */
#include "common.h"

/** @brief Field actor record (0x54 bytes); only the fields this handler touches are named. */

typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 pad8[0x16 - 0x8];
    s16 unk16;
    u8 pad18[0x1C - 0x18];
    s32 unk1C;
    u8 unk20;
    u8 unk21;
    u8 pad22[0x27 - 0x22];
    u8 unk27;
    u8 pad28[0x2A - 0x28];
    s16 unk2A;
    u8 pad2C[0x34 - 0x2C];
    u8 unk34;
    u8 unk35;
    u8 unk36;
    s8 unk37;
    s8 unk38;
    u8 unk39;
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldRec;

/** @brief g_field_object_states slot record (stride 0x23C). */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x170 - 0x10];
    u8 unk170;
    u8 pad171[0x178 - 0x171];
    u32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Slot23C;

/** @brief g_field_player_records object entry (stride 0x268). */
typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 pad2[0x268 - 0x2];
} Entry268;

/** @brief g_field_resource_actions animation record (stride 0x190); unk5A is written as both a u16 and its low byte. */
typedef struct
{
    u16 unk0;
    u8 pad2[0x8 - 0x2];
    u16 unk8;
    u8 padA[0x58 - 0xA];
    u16 unk58;
    union
    {
        u16 h;
        struct
        {
            u8 lo;
            u8 hi;
        } b;
    } unk5A;
    u16 unk5C;
    u16 unk5E;
    u8 pad60[0x190 - 0x60];
} Anim190;

extern Anim190 g_field_resource_actions[];

void field_start_actor_animation();
void field_restart_actor_animation();
s32 func_800839F8(s32 arg0, s32 arg1);
s32 func_80083EEC();
void func_8008A9D8(s32 arg0, s32 arg1, s32 arg2);
s32 func_8008AABC(s32 a, s32 b);
void func_8008BC5C(FieldRec *rec);
void field_prepare_actor_action(FieldRec *rec);
s32 func_80091728(u8 index, s32 kind, FieldRec *rec);
s32 func_80091914(FieldRec *rec, u8 index);
void field_restart_sequence_animation();
void func_800A2DD8();

/** @brief Program the animation record for object @p idx (fields 0x58..0x5E). */
#define SET_ANIM(idx, v58, v5C, v5E)                                  \
    g_field_resource_actions[idx].unk5A.h &= 0xFBFF;                                \
    g_field_resource_actions[idx].unk58 = v58;                                      \
    g_field_resource_actions[idx].unk5A.b.lo = 0xFF;                                \
    g_field_resource_actions[idx].unk5C = v5C;                                      \
    g_field_resource_actions[idx].unk5E = v5E;                                      \
    g_field_resource_actions[idx].unk5A.h &= 0xFCFF;

/** @brief Interpolated step offset (unk37..unk38 scaled by unk34/unk35), in 1/256 units. */
#define STEP_OFFSET(rec) \
    ((rec->unk37 + (rec->unk38 - rec->unk37) * rec->unk34 / rec->unk35) << 8)

/**
 * @brief Per-frame state handler for a field actor's opcode 0x86 / trigger-kind states.
 *
 * With no pending flags in unk1C, first resolves the 0x3D transition when the
 * current animation matches, then dispatches on the opcode (unk21 & 0x7F) by
 * trigger kind (func_80091728 kinds 3, 1/0, 2), programming the g_field_resource_actions
 * animation record and queueing the follow-up state via field_prepare_actor_action.
 *
 * @param rec Field actor record.
 * @return Never set; the declared non-void return keeps v0 live at the epilogue,
 *         which is what the original codegen shows (all exits are bare returns).
 * @see decomp.me (100%) TODO
 */
s32 func_80092C98(FieldRec *rec)
{
    s32 targets;
    s32 tmp;
    s32 anim;
    s32 anim_id;
    s32 index;

    if (rec->unk1C & 0x1FF)
    {
        return;
    }
    if (rec->unk2A == 0x86)
    {
        tmp = rec->unk21 & 0x7F;
        if (tmp == 0x3D)
        {
            anim = func_80091914(rec, rec->unk3A);
            if (g_field_resource_actions[rec->unk3A].unk8 == tmp && anim == 0x185)
            {
                rec->unk2A = anim;
                rec->unk4 -= STEP_OFFSET(rec);
                field_prepare_actor_action(rec);
                func_800A2DD8(rec->unk3A);
                rec->unk2A = 0x9B;
                return;
            }
            else if (g_field_resource_actions[rec->unk3A].unk0 == 0x3D && anim == 0x85)
            {
                rec->unk2A = anim;
                rec->unk4 -= STEP_OFFSET(rec);
                field_prepare_actor_action(rec);
                func_800A2DD8(rec->unk3A);
                rec->unk2A = 0x9B;
                return;
            }
        }
    }
    if (func_80091728(rec->unk3A, 3, rec) != 0)
    {
        switch (rec->unk21 & 0x7F)
        {
        case 0x2F:
        case 0x44:
            rec->unk2A = 0x885;
            field_prepare_actor_action(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x3E:
            rec->unk2A = 0xA85;
            field_prepare_actor_action(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x38:
            rec->unk2A = 0xA85;
            field_prepare_actor_action(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x3A:
            SET_ANIM(rec->unk3A, 0x4F, 0x25, 0);
            rec->unk2A = 0xB85;
            field_prepare_actor_action(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x39:
            SET_ANIM(rec->unk3A, 0x4F, 0x25, 0);
            rec->unk2A = 0xB85;
            field_prepare_actor_action(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x34:
            SET_ANIM(rec->unk3A, 0x51, 0x27, 0);
            rec->unk2A = 0xB85;
            field_prepare_actor_action(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x8:
        case 0x3B:
        case 0x3C:
        case 0x3D:
            if (rec->unk27 < 3)
            {
                return;
            }
            rec->unk21 = (rec->unk21 & 0x80) | 0x49;
            rec->unk4 -= STEP_OFFSET(rec);
            field_restart_actor_animation(rec);
            func_800A2DD8(rec->unk3A);
            rec->unk2A = 0x96;
            rec->unk16 = 1;
            rec->unk34 = 1;
            rec->unk35 = 1;
            return;
        case 0x35:
            if ((((Slot23C *)g_field_object_states)[rec->unk3A].unk178 >> 1) & 1)
            {
                ((Slot23C *)g_field_object_states)[((Slot23C *)g_field_object_states)[rec->unk3A].unk170].unkC &= ~0x2000;
                rec->unk2A = 0;
                field_restart_sequence_animation(rec);
                tmp = func_800839F8(rec->unk3A, 0);
                if (tmp != -1)
                {
                    if (func_8008AABC(rec->unk3A, ((Slot23C *)g_field_object_states)[rec->unk3A].unk170) != 0)
                    {
                        if (((Entry268 *)g_field_player_records)[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(rec->unk3A, ((Slot23C *)g_field_object_states)[rec->unk3A].unk170, 0xD);
                        }
                        else
                        {
                            func_8008A9D8(rec->unk3A, ((Slot23C *)g_field_object_states)[rec->unk3A].unk170, 0xC);
                        }
                        index = rec->unk3A;
                        anim_id = 0x64;
                        if (((Entry268 *)g_field_player_records)[index].unk1 == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (((Entry268 *)g_field_player_records)[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(((Slot23C *)g_field_object_states)[rec->unk3A].unk170, rec->unk3A, 0x18);
                        }
                        else
                        {
                            func_8008A9D8(((Slot23C *)g_field_object_states)[rec->unk3A].unk170, rec->unk3A, 0x17);
                        }
                        index = rec->unk3A;
                        anim_id = 0x65;
                        if (((Entry268 *)g_field_player_records)[index].unk1 == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (func_80083EEC(index, tmp, anim_id) != 0)
                    {
                        targets = ((Slot23C *)g_field_object_states)[rec->unk3A].unk170;
                        field_start_actor_animation(tmp, 1, &targets);
                    }
                    func_800A2DD8(rec->unk3A);
                }
                func_8008BC5C(rec);
            }
            return;
        default:
            return;
        }
    }
    else if (func_80091728(rec->unk3A, 1, rec) != 0 || func_80091728(rec->unk3A, 0, rec) != 0)
    {
        tmp = func_80091728(rec->unk3A, 1, rec) != 0;
        switch (rec->unk21 & 0x7F)
        {
        case 0x25:
        {
            s32 base;
            s32 actor_offset;
            s32 track_offset;
            s32 offset;
            base = (s32)g_field_resource_actions;
            track_offset = tmp * 8;
            actor_offset = rec->unk3A * 0x190;
            offset = track_offset + actor_offset + base;
            if (*(u16 *)offset == 8 || *(u16 *)offset == 0x3C)
            {
                rec->unk2A = 0x985;
                field_prepare_actor_action(rec);
                func_800A2DD8(rec->unk3A);
            }
            break;
        }
        case 0x31:
        {
            s32 base;
            s32 track_offset;
            s32 actor_offset;
            s32 offset;
            base = (s32)g_field_resource_actions;
            track_offset = tmp * 8;
            actor_offset = rec->unk3A * 0x190;
            offset = track_offset + actor_offset + base;
            if (*(u16 *)offset == 8)
            {
                SET_ANIM(rec->unk3A, 0x3C, 0, 1);
                rec->unk2A = 0xB85;
                field_prepare_actor_action(rec);
                func_800A2DD8(rec->unk3A);
            }
            break;
        }
        }
    }
    else if (func_80091728(rec->unk3A, 2, rec) != 0)
    {
        if ((rec->unk21 & ~0x80) == 0x34)
        {
            SET_ANIM(rec->unk3A, 0x50, 0x26, 0);
            rec->unk2A = 0xB85;
            field_prepare_actor_action(rec);
            func_800A2DD8(rec->unk3A);
        }
        if ((rec->unk21 & ~0x80) == 0x35)
        {
            if ((((Slot23C *)g_field_object_states)[rec->unk3A].unk178 >> 1) & 1)
            {
                ((Slot23C *)g_field_object_states)[((Slot23C *)g_field_object_states)[rec->unk3A].unk170].unkC &= ~0x2000;
                rec->unk2A = 0;
                field_restart_sequence_animation(rec);
                tmp = func_800839F8(rec->unk3A, 0);
                if (tmp != -1)
                {
                    if (func_8008AABC(rec->unk3A, ((Slot23C *)g_field_object_states)[rec->unk3A].unk170) != 0)
                    {
                        if (((Entry268 *)g_field_player_records)[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(rec->unk3A, ((Slot23C *)g_field_object_states)[rec->unk3A].unk170, 0xD);
                        }
                        else
                        {
                            func_8008A9D8(rec->unk3A, ((Slot23C *)g_field_object_states)[rec->unk3A].unk170, 0xC);
                        }
                        index = rec->unk3A;
                        anim_id = 0x64;
                        if (((Entry268 *)g_field_player_records)[index].unk1 == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (((Entry268 *)g_field_player_records)[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(((Slot23C *)g_field_object_states)[rec->unk3A].unk170, rec->unk3A, 0x18);
                        }
                        else
                        {
                            func_8008A9D8(((Slot23C *)g_field_object_states)[rec->unk3A].unk170, rec->unk3A, 0x17);
                        }
                        index = rec->unk3A;
                        anim_id = 0x65;
                        if (((Entry268 *)g_field_player_records)[index].unk1 == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (func_80083EEC(index, tmp, anim_id) != 0)
                    {
                        targets = ((Slot23C *)g_field_object_states)[rec->unk3A].unk170;
                        field_start_actor_animation(tmp, 1, &targets);
                    }
                }
            }
            func_800A2DD8(rec->unk3A);
        }
    }
}

/* field_actor_action_runtime: Validate pending actions, clear completed state, and advance actor sequences. */

/* field_actor_pending_actions */
/* func_80093AB8 */
#include "common.h"

/** @brief Actor fields used to track and reset pending action state. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x1C - 8];
    s32 unk1C;
    u8 unk20, unk21;
    u8 pad22[8];
    u16 unk2A;
    u8 pad2C[4];
    u16 unk30;
    u8 pad32[8];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Actor;
/** @brief Action flags and mode bytes within a 0x23C-byte object slot. */
typedef struct
{
    u8 pad0[12];
    s32 unkC;
    u8 pad10[0x16F - 0x10];
    u8 unk16F;
    u8 pad170[0x18D - 0x170];
    u8 unk18D;
    u8 pad18E[0x23C - 0x18E];
} Slot;
/** @brief Party record containing the action-type byte at offset one. */
typedef struct
{
    u8 unk0, unk1;
    u8 pad2[0x268 - 2];
} Party;

void field_restart_actor_animation_reverse(Actor *);
s32 field_get_next_animation_frame_count(Actor *);
s32 func_800A29F8(s32, s32, s32);
void func_800A2DD8();
/**
 * @brief Update pending actor actions and clear stale action state.
 * @param input Actor whose object slot and pending-action counter are checked.
 * @return One when the actor enters state 0x95; zero otherwise.
 */
s32 func_80093AB8(Actor *input)
{
    Actor *actor = input;
    s32 selection;
    s32 clear_mask;
    s32 flags;
    u16 retry_count;
    u16 count;
    u8 object_index;
    s32 mode;
    Slot *slot;
    Slot *base;
    Slot *reset_slot;

    if (actor->unk1C & 0x1FF)
    {
        ((Slot *)g_field_object_states)[actor->unk3A].unkC &= 0xFFFF7FFF;
        actor->unk30 = (u16)(actor->unk30 + 1);
        return 0;
    }
    selection = func_800A29F8(actor->unk3A, ((u8)actor->unk21 >> 7) ^ 1, 1);
    base = ((Slot *)g_field_object_states);
    slot = &base[actor->unk3A];
    if (((slot->unk16F == 2) || (actor->unk30 != 0)) && (actor->unk4 == 0))
    {
        if (!(actor->unk1C & 0x1FF))
        {
            flags = slot->unkC;
            if (!(flags & 0x400))
            {
                slot->unkC = (s32)(flags | 0x8000);
            }
        }
        if (selection != 3)
        {
            if (selection < 4)
            {
                if (selection == 2)
                {
                    count = actor->unk30;
                    if (count < 5U)
                    {
                        actor->unk30 = (u16)(count + 1);
                    }
                    if (field_get_next_animation_frame_count(actor) != 0)
                    {
                        retry_count = actor->unk30;
                        if (retry_count < 5U)
                        {
                            if ((u8)actor->unk3A < 2U)
                            {
                                if (((Party *)g_field_player_records)[actor->unk3A].unk1 == 0xA)
                                {
                                    if (retry_count >= 3U)
                                    {
                                        goto cancel_pending;
                                    }
                                    goto check_mode;
                                }
                                goto check_mode;
                            }
                            goto check_mode;
                        }
                    }
                cancel_pending:
                    func_800A2DD8(actor->unk3A);
                    clear_mask = 0xFFFF7FFF;
                    actor->unk30 = 0U;
                    ((Slot *)g_field_object_states)[actor->unk3A].unk18D = 0;
                    reset_slot = &((Slot *)g_field_object_states)[actor->unk3A];
                    goto reset_actor;
                }
                goto clear_pending_counter;
            }
        }
    clear_pending_counter:
        ((Slot *)g_field_object_states)[actor->unk3A].unk18D = 0;
        actor->unk30 = 0U;
    }
check_mode:
    object_index = actor->unk3A;
    mode = ((Slot *)g_field_object_states)[object_index].unk16F;
    if (mode == 3)
    {
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7 &&
            selection != 8 && selection != 9 && selection != 10)
        {
            func_800A2DD8(object_index);
            clear_mask = 0xFFFF7FFF;
            ((Slot *)g_field_object_states)[actor->unk3A].unk18D = 0;
            actor->unk30 = 0;
            reset_slot = &((Slot *)g_field_object_states)[actor->unk3A];
        reset_actor:
            flags = reset_slot->unkC;
            flags &= clear_mask;
            reset_slot->unkC = flags;
            field_restart_actor_animation_reverse(actor);
            actor->unk2A = 0x95;
            actor->unk20 = 0x14;
            return 1;
        }
    }
    else
    {
        if ((s32)mode >= 3)
        {
            if ((s32)mode < 11)
            {
                if ((s32)mode >= 8)
                {
                    if (selection != 4 && selection != 6 && selection != 5 && selection != 7)
                    {
                        func_800A2DD8(object_index);

                        ((Slot *)g_field_object_states)[actor->unk3A].unk18D = 0;
                        actor->unk30 = 0;
                        ((Slot *)g_field_object_states)[actor->unk3A].unkC &= 0xFFFF7FFF;
                    }
                }
            }
        }
    }
    return 0;
}

/* func_80093EB4 */
#include "common.h"
typedef struct { u8 pad0[0x1C]; s32 unk1C; u8 pad20[0xA]; s16 unk2A; u8 pad2C[2]; u16 unk2E; u8 pad30[0xA]; u8 unk3A; } FieldRecord;
typedef struct { u8 pad0[0x4A]; s16 unk4A; u8 pad4C[0x128]; s32 unk174; u8 pad178[1]; u8 unk179; u8 pad17A[0xC2]; } FieldState;
typedef struct { u8 pad0[0x228]; u8 unk228; u8 pad229[0x11]; u8 unk23A; u8 pad23B[9]; } ActorSlot;
void func_8008A678(); void field_update_sequence_actor_binding(); void func_800A2DD8();

/**
 * @brief Clear field state for a record after validating its linked actor slot.
 * @param arg0 Field record whose state is updated.
 */
void func_80093EB4(FieldRecord *arg0)
{
    ActorSlot *slot;
    FieldState *state;
    FieldState *states = (FieldState *)((Slot *)g_field_object_states);
    u8 index;
    u8 slotIndex;
    s32 gate;

    index = arg0->unk3A;
    state = &states[index];
    slotIndex = state->unk179;
    if (slotIndex == 0xFF)
    {
        if (arg0->unk2E != 0)
        {
            return;
        }
        state->unk174 &= ~0x1800;
        arg0->unk2A = 0;
        arg0->unk1C &= ~0x800;
        states[arg0->unk3A].unk4A = 0;
        if ((u8)arg0->unk3A < 2)
        {
            func_800A2DD8(arg0->unk3A);
        }
    }
    else
    {
        gate = arg0->unk2E;
        slot = &((ActorSlot *)g_field_actor_slots)[slotIndex];
        if (gate != 0 || (slot->unk23A != 0 && slot->unk228 == index))
        {
            return;
        }
        gate = index;
        func_8008A678(gate, index);
        arg0->unk2A = 0;
        gate = ~0x1800;
        states[arg0->unk3A].unk174 &= gate;
        if (slot->unk228 == arg0->unk3A)
        {
            field_update_sequence_actor_binding(arg0, 1);
        }
        arg0->unk1C &= ~0x800;
        states[arg0->unk3A].unk4A = 0;
        if ((u8)arg0->unk3A < 2)
        {
            func_800A2DD8(arg0->unk3A);
        }
    }
}

/* func_8009403C */
#include "common.h"
#include "vector.h"

/** @brief Accessed fields of an actor sequence record. */
typedef struct
{
    u8 pad00[0x16];
    s16 motion_divisor;
    u8 pad18[4];
    u32 sequence_flags;
    u8 pad20;
    u8 facing_flags;
    u8 pad22[8];
    s16 sequence_state;
    u8 pad2c[2];
    u16 sequence_delay;
    u8 pad30[6];
    s8 motion_remainder;
    u8 pad37[3];
    u8 object_id;
    u8 resource_id;
} FieldSequenceRecord;

/** @brief Sequence state within one 0x23C-byte actor slot. */
typedef struct
{
    u8 pad00[0xC];
    u32 object_flags;
    u8 pad10[0x44 - 0x10];
    s32 script_position;
    u8 pad48[0x171 - 0x48];
    u8 command_delay;
    u8 pad172[2];
    u32 track_flags;
    u8 pad178[3];
    u8 repeat_state;
    u8 pad17c[0x23C - 0x17C];
} FieldSequenceSlot;

/** @brief Script bank selector within a 0x268-byte object record. */
typedef struct
{
    u8 pad00;
    u8 bank;
    u8 pad02[0x268 - 2];
} FieldSequenceBank;

/** @brief Motion multiplier within a 0x48-byte object record. */
typedef struct
{
    u8 pad00[0x2E];
    u8 scale;
    u8 pad2f[0x48 - 0x2F];
} FieldSequenceMotion;

/** @brief Mode byte within one 0x14-byte resource entry. */
typedef struct
{
    u8 pad00[8];
    u8 mode;
    u8 pad09[0x14 - 9];
} FieldSequenceResource;

extern FieldSequenceResource g_field_resource_entries[];
extern u8 g_field_actor_sequence_data[];
s32 field_object_has_active_actor_tracks(u8);
void field_stop_actor_animations_for_object(FieldSequenceRecord *, s32);
void field_restart_actor_animation();
void func_8008A678(s32);
void field_update_sequence_actor_binding();
s32 field_execute_actor_sequence();
s32 field_resolve_actor_movement();
void func_800A2DD8();

/**
 * @brief Advance an actor sequence and apply its pending horizontal motion.
 * @param record Actor sequence record to update.
 * @param sequence_index Script row within the object's selected bank.
 * @note Command 0xF1 advances the cursor; 0xEF handles sequence completion.
 */
void func_8009403C(FieldSequenceRecord *record, s32 sequence_index)
{
    Vec3i *scratch = (Vec3i *)0x1F800000;
    FieldSequenceSlot *slot_base;
    FieldSequenceSlot *slot;
    FieldSequenceSlot *timer_slot, *reset_slot, *final_slot;
    FieldSequenceSlot *final_base;
    u8 *active_scripts;
    FieldSequenceBank *active_banks;
    s32 active_address, active_position;
    s32 first_position;
    s32 bank;
    FieldSequenceBank *banks;
    FieldSequenceMotion *motion;
    u8 *scripts;
    s32 object;
    s32 position;
    s32 row_offset;
    s32 row_address;
    s32 combined_address;
    s32 amount;
    u8 delay;
    u8 command;

    if (record->sequence_delay != 0)
    {
        goto apply_motion;
    }
    slot_base = ((FieldSequenceSlot *)((Slot *)g_field_object_states));
    timer_slot = &slot_base[record->object_id];
    delay = timer_slot->command_delay;
    if (delay != 0)
    {
        timer_slot->command_delay = delay - 1;
        if (slot_base[record->object_id].command_delay != 0)
        {
            goto apply_motion;
        }
    }
    if (field_object_has_active_actor_tracks(record->object_id) != 0)
    {
        object = record->object_id;
        active_position = slot_base[object].script_position;
        if (active_position == 1)
        {
            goto apply_motion;
        }
        active_scripts = g_field_actor_sequence_data;
        do
        {
            active_scripts = g_field_actor_sequence_data;
        } while (0);
        active_banks = ((FieldSequenceBank *)((Party *)g_field_player_records));
        active_address = (sequence_index << 5) + active_banks[object].bank * 0x300;
        active_address += (s32)active_scripts;
        command = *(u8 *)(active_address + active_position);
        if (command == 0xFF || command == 0xF1)
        {
            goto apply_motion;
        }
    }
    reset_slot = &slot_base[record->object_id];
    if (reset_slot->script_position == 1)
    {
        reset_slot->track_flags &= ~0x1800;
        slot_base[record->object_id].repeat_state = 0;
    }
    scripts = g_field_actor_sequence_data;
    banks = ((FieldSequenceBank *)((Party *)g_field_player_records));
    object = record->object_id;

    do
    {
        bank = banks[object].bank;
        row_offset = sequence_index << 5;
        row_address = bank << 1;
        row_address += bank;
        row_address <<= 8;
        combined_address = row_offset + row_address;
        row_address = combined_address;
    } while (0);
    slot = &slot_base[object];
    first_position = slot->script_position;
    row_address += (s32)scripts;
    row_address += first_position;
    if (*(u8 *)row_address == 0xF1)
    {
        slot->script_position = first_position + 1;
    }
    object = record->object_id;
    do
    {
        bank = banks[object].bank;
        row_address = bank << 1;
        row_address += bank;
        row_address <<= 8;
        combined_address = row_offset + row_address;
        row_address = combined_address;
    } while (0);
    slot = &slot_base[object];
    position = slot->script_position;
    row_address += (s32)scripts;
    row_address += position;
    if (*(u8 *)row_address == 0xEF)
    {
        if (slot->repeat_state == 0)
        {
            func_8008A678(object);
            field_stop_actor_animations_for_object(record, 1);
            goto stop_sequence;
        }
        slot->script_position = position + 1;
        return;
    }
    if (field_execute_actor_sequence(record, sequence_index) != 0)
    {
        func_8008A678(record->object_id);
    stop_sequence:
        record->sequence_state = 0;
        field_update_sequence_actor_binding(record, 1);
        record->sequence_flags &= ~0x800;
        if (record->object_id < 2U)
        {
            func_800A2DD8(record->object_id);
        }
        slot_base[record->object_id].object_flags &= ~0x4000;
        slot_base[record->object_id].object_flags &= 0xFFFF7FFF;
        return;
    }
    field_restart_actor_animation(record);
    record->sequence_flags |= 0x800;
apply_motion:
    amount = (s8)record->motion_remainder / (s16)record->motion_divisor;
    motion = &((FieldSequenceMotion *)g_field_object_parts)[record->object_id];
    record->motion_remainder = (u8)record->motion_remainder - amount;
    if (record->facing_flags & 0x80)
    {
        scratch->x = ((amount << 8) * motion->scale) >> 6;
    }
    else
    {
        scratch->x = (-(amount << 8) * motion->scale) >> 6;
    }
    scratch->z = 0;
    scratch->y = 0;
    field_resolve_actor_movement(record, scratch, 1);
    if (g_field_resource_entries[record->resource_id].mode == 0)
    {
        final_base = ((FieldSequenceSlot *)((Slot *)g_field_object_states));
        final_slot = &final_base[record->object_id];
        final_slot->track_flags &= ~0x4000;
    }
}

/* field_actor_displacement: Apply actor displacement, follow leader history, and refresh collision contact. */

/* func_80094508 */
#include "common.h"

/** @brief Partial actor state used by the scaled movement update. */
typedef struct
{
    u8 pad0[0x16];
    s16 unk16;
    u8 pad18[0x2A - 0x18];
    s16 unk2A;
    u8 pad2C[0x2E - 0x2C];
    u16 unk2E;
    u8 pad30[0x36 - 0x30];
    u8 unk36;
    u8 pad37[0x3A - 0x37];
    u8 unk3A;
} Blk80094508_FieldActorState;

/** @brief Actor-part scaling bytes in the 0x48-byte definition table. */
typedef struct
{
    u8 pad0[0x2E];
    u8 unk2E;
    u8 pad2F[0x33 - 0x2F];
    u8 unk33;
    u8 pad34[0x48 - 0x34];
} Blk80094508_FieldActorPartDef;

void field_update_actor_movement_animation();
s32 field_resolve_actor_movement();

/**
 * @brief Update actor movement and clear selected states when the transform fails.
 * @param arg0 Actor state to update.
 * @param arg1 Horizontal scale input.
 * @param arg2 Vertical scale input.
 * @param arg3 Depth scale input.
 * @return Unspecified value; callers do not consume the result.
 */
s32 func_80094508(Blk80094508_FieldActorState *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_lo;
    Blk80094508_FieldActorPartDef *part;
    s32 *out;
    s16 state;

    out = (s32 *)0x1F800000;
    if (arg0->unk2E == 0)
    {
        arg0->unk2A = 0;
    }
    else
    {
        field_update_actor_movement_animation(arg0, arg1, arg3);
        temp_lo = (s8)arg0->unk36 / arg0->unk16;
        arg0->unk36 = (u8)arg0->unk36 - temp_lo;
        part = &((Blk80094508_FieldActorPartDef *)g_field_object_parts)[arg0->unk3A];
        out[0] = (temp_lo * arg1 * part->unk2E) >> 6;
        out[1] = (arg2 * part->unk33) >> 6;
        out[2] = (temp_lo * arg3 * part->unk2E) >> 6;
        if (field_resolve_actor_movement(arg0, out, 0) == 0)
        {
            state = arg0->unk2A;
            if (state == 0x8B || state == 0xAC || state == 0x8C || state == 0xB0 || state == 0xB1)
            {
                arg0->unk2A = 0;
            }
        }
    }
}

/* func_80094690 */
#include "common.h"
#include "vector.h"

/**
 * @brief Field record fields used by the scaled position query.
 */
typedef struct Record94690
{
    u8 pad0[0x20];
    u8 scale;
    u8 pad21[0x2A - 0x21];
    s16 value;
} Record94690;

s32 field_resolve_actor_movement();

/**
 * @brief Tests a scaled X/Z displacement for a field record.
 *
 * Builds a scratchpad vector from the supplied X/Z components and the byte
 * scale at record offset 0x20. A failed query clears the record halfword at
 * offset 0x2A.
 *
 * @param record Record supplying the scale and result halfword.
 * @param x X displacement before scaling.
 * @param z Z displacement before scaling.
 * @note 100% match. Reusing one scaled-value local for both products
 *       reproduces the target value web and register reuse.
 */
void func_80094690(Record94690 *record, s32 x, s32 z)
{
    Vec3i *vector;
    s32 scaled;

    scaled = x * record->scale;
    vector = (Vec3i *)0x1F800000;
    vector->y = 0;
    vector->x = scaled;
    scaled = z * record->scale;
    vector->z = scaled;
    if (field_resolve_actor_movement(record, vector, 0, scaled) == 0)
    {
        record->value = 0;
    }
}

/* func_800946FC */
#include "common.h"
#include "field_types.h"
#include "sdk/inline_c.h"

/* Apply the matching GTE instruction encodings after the SDK macros. */
#include "sdk/gte_dmpsx_compat.h"

/** @brief Partial field record used by the history-following update. */
typedef struct
{
    s32 x, y, z;
    u8 padc[0x21 - 0xC];
    u8 state;
    u8 pad22[0x2A - 0x22];
    s16 unk2a;
    u8 pad2c[2];
    s16 unk2e;
    u8 pad30[3];
    u8 unk33;
    u8 pad34[6];
    u8 slot;
    u8 resource;
    u8 pad3c[0x54 - 0x3C];
} FieldFollowRecord;

/** @brief Slot history with packed X/Z points and the current follow index. */
typedef struct
{
    u8 pad0[0x6C];
    Vec2s points[48];
    u8 pad12c[0x16E - 0x12C];
    u8 history_index;
    u8 pad16f[0x23C - 0x16F];
} FieldFollowSlot;

/** @brief Partial resource descriptor exposing its behavior flags. */
typedef struct
{
    u8 pad0[0x10];
    u32 flags;
} FieldFollowResource;

void field_update_actor_movement_animation();
extern void field_restart_sequence_animation();
s32 field_resolve_actor_movement();

/**
 * @brief Follow stored leader positions or restore the record's idle behavior.
 * @param record Follower record whose movement and history index are updated.
 * @note Packed history addressing and scratchpad GTE operations preserve codegen.
 */
void func_800946FC(FieldFollowRecord *record)
{
    VECTOR *delta = (VECTOR *)0x1F800010;
    VECTOR *squares = (VECTOR *)0x1F800000;
    FieldFollowSlot *slots;
    FieldFollowSlot *slot;
    s32 distance;
    u8 index;
    u8 next;
    s32 state;

    delta->vy = 0;
    delta->vx = (((FieldFollowRecord *)g_field_actors)[0].x - record->x) / 256;
    delta->vz = (((FieldFollowRecord *)g_field_actors)[0].z - record->z) / 256;
    gte_ldlvl(delta);
    gte_sqr0();
    gte_stlvnl(squares);
    index = record->slot;
    distance = squares->vx + squares->vz;
    if (((index + 1) * 2000 < distance) &&
        (slots = ((FieldFollowSlot *)g_field_object_states), slot = &slots[index], next = slot->history_index, next < 47))
    {
        slot->history_index = next + 1;
        /* Fold the slot and point indices together before the four-byte stride. */
        delta->vx =
            (*(s16 *)((u8 *)slots +
                      (((FieldFollowRecord *)g_field_actors)[0].slot * 0x8F + slots[record->slot].history_index) * 4 + 0x6C)
             << 8) -
            record->x;
        delta->vz =
            (*(s16 *)((u8 *)slots +
                      (((FieldFollowRecord *)g_field_actors)[0].slot * 0x8F + slots[record->slot].history_index) * 4 + 0x6E)
             << 8) -
            record->z;
        gte_ldlvl(delta);
        gte_sqr12();
        gte_stlvnl(squares);
        if (squares->vx + squares->vz > 384 &&
            !(((FieldFollowResource *)g_field_resource_entries)[record->resource].flags & 1))
        {
            record->unk33 = 1;
        }
        else
        {
            record->unk33 = 0;
        }
    }
    else
    {
        if (((FieldFollowResource *)g_field_resource_entries)[record->resource].flags & 1)
        {
            record->state &= 0x80;
        }
        else
        {
            state = record->state & 0x80;
            state += 2;
            record->state = state;
        }
        record->unk2e = 1;
        field_restart_sequence_animation(record);
        record->unk33 = 0;
        goto clear_state;
    }
    field_update_actor_movement_animation(record, delta->vx, delta->vz);
    if (field_resolve_actor_movement(record, delta, 0) == 0)
    {
        record->unk33 = 0;
    clear_state:
        record->unk2a = 0;
    }
}

/* func_800949CC */
#include "common.h"

typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x18 - 0x10];
    s32 unk18;
} FieldTrackEntry;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x23A - 0x25];
    u8 unk23A;
    u8 pad23B[0x244 - 0x23B];
} FieldTrackActor;

typedef struct
{
    u8 pad0[0x20];
    u8 scale;
    u8 pad21[0x2A - 0x21];
    s16 state;
    u8 pad2C[0x2E - 0x2C];
    u16 transform_mode;
    u8 pad30[0x3A - 0x30];
    u8 track;
} FieldActorRecord;

s32 field_resolve_actor_movement();

/**
 * @brief Validate an actor's active track state or submit a scaled scratchpad vector.
 * @param record Actor record to update.
 * @param x X component used by the transform path.
 * @param y Y component used by the transform path.
 * @param z Z component used by the transform path.
 */
void func_800949CC(FieldActorRecord *record, s32 x, s32 y, s32 z)
{
    s32 offset;
    s32 key;
    s32 selector;
    u8 *first_base;
    u8 *second_base;
    u8 *third_base;
    FieldTrackActor *actors;
    FieldTrackActor *actor;
    s32 *scratch;

    scratch = (s32 *)0x1F800000;
    if (record->transform_mode == 0)
    {
        first_base = (u8 *)((FieldTrackEntry *)g_field_actor_bindings);
        if ((u8)record->track < 2U)
        {
            offset = record->track * 0x1C;
        }
        else
        {
            offset = 0x38;
        }
        selector = record->track;
        key = *(s32 *)(first_base + offset + 0xC);
        if (key == selector)
        {
            actors = ((FieldTrackActor *)g_field_actor_slots);
            second_base = (u8 *)((FieldTrackEntry *)g_field_actor_bindings);
            if ((u32)(key & 0xFF) < 2U)
            {
                offset = key * 0x1C;
            }
            else
            {
                offset = 0x38;
            }
            actor = actors + *(s32 *)(second_base + offset + 0x18);
            if (actor->unk24 != 0)
            {
                actors = ((FieldTrackActor *)g_field_actor_slots);
                third_base = (u8 *)((FieldTrackEntry *)g_field_actor_bindings);
                if ((u8)record->track < 2U)
                {
                    offset = record->track * 0x1C;
                }
                else
                {
                    offset = 0x38;
                }
                actor = actors + *(s32 *)(third_base + offset + 0x18);
                if (actor->unk23A == 0)
                {
                    record->state = 0;
                }
            }
            else
            {
                record->state = 0;
            }
        }
        else
        {
            record->state = 0;
        }
    }
    else
    {
        scratch[0] = x * record->scale;
        scratch[1] = y * record->scale;
        scratch[2] = z * record->scale;
        field_resolve_actor_movement(record, scratch, 0);
    }
}

/* func_80094B5C */
#include "common.h"

typedef struct
{
    u8 pad0[0x4];
    s32 unk4;  /* 0x04 */
    u8 pad8[0x20 - 0x8];
    u8 unk20;  /* 0x20 */
    u8 pad21[0x26 - 0x21];
    u8 unk26;  /* 0x26 */
    u8 pad27[0x2A - 0x27];
    s16 unk2A; /* 0x2A */
    u8 pad2C[0x30 - 0x2C];
} Struct80094B5C;

/**
 * @brief Advances an actor's countdown and scrolls its 0x04 offset field.
 *
 * Decrements the byte counter at @c unk26 and zeroes @c unk2A when it reaches
 * 0. Then, when @p flag is set, subtracts @c unk20 << 8 from @c unk4; otherwise
 * adds it, and if the sum is non-negative resets @c unk4 and @c unk2A to 0.
 *
 * @param a0 Actor record to update.
 * @param flag Nonzero subtracts the delta from @c unk4; zero adds it (with the
 *             non-negative reset).
 */
void func_80094B5C(Struct80094B5C *a0, s32 flag)
{
    s8 v;

    v = a0->unk26 - 1;
    a0->unk26 = v;
    if (v == 0)
    {
        a0->unk2A = 0;
    }

    if (flag != 0)
    {
        a0->unk4 -= a0->unk20 << 8;
    }
    else
    {
        s32 w = a0->unk4 + (a0->unk20 << 8);
        a0->unk4 = w;
        if (w >= 0)
        {
            a0->unk4 = 0;
            a0->unk2A = 0;
        }
    }
}

/* field40 */
#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x14];
    u8 unk20;
} UnkStruct21;

void func_80094BC4(UnkStruct21 *arg0, s32 arg1, s32 arg2)
{
    u8 temp_v0;

    temp_v0 = arg0->unk20;
    arg0->unk0 = arg0->unk0 + (arg1 * temp_v0);
    arg0->unk8 = arg0->unk8 + (arg2 * temp_v0);
}

/* func_80094C00 */
#include "common.h"

/** @brief Position, speed, and slot-index prefix of a field actor. */
typedef struct
{
    s32 x, y, z;
    u8 pad_c[0x14];
    u8 speed;
    u8 pad_21[0x19];
    u8 slot;
} FieldMovingActor;
/** @brief Visual kind byte in a 0x48-byte field object record. */
typedef struct
{
    u8 pad[0x2E];
    u8 kind;
    u8 tail[0x19];
} FieldObjectVisualKind;
/** @brief Collision result fields in a 0x23C-byte actor slot. */
typedef struct
{
    u8 pad[0x176];
    s16 height;
    u8 pad178[0x24];
    s32 contact, surface;
    u8 pad1a4[0x98];
} FieldActorCollisionResult;
/** @brief Scratchpad collision request and resolver output. */
typedef struct
{
    s32 x, y, z, dx, dy, dz, height, contact, surface;
    s16 radius, depth;
    union
    {
        s32 flags;
        struct
        {
            unsigned step : 16;
            unsigned bit16 : 1;
            unsigned bit17 : 1;
            unsigned high : 14;
        } bits;
    } mode;
} FieldActorCollisionMover;
/** @brief Map dimensions used to validate fixed-point actor coordinates. */
typedef struct
{
    s16 width;
    u16 height;
} FieldActorCollisionBounds;

extern s32 func_8005B6AC(FieldActorCollisionMover *);

/**
 * @brief Move an actor by its speed and refresh its collision contact and height.
 * @param actor Actor position, speed, and destination slot index.
 * @param dx Horizontal direction or displacement multiplier.
 * @param dz Depth direction or displacement multiplier.
 * @note Uses collision scratchpad memory at 0x1F800000 and map bounds at 0x801ED400.
 * @note 100% match with GCC 2.7.2 CDK: 169 instructions, 676 bytes.
 */
void func_80094C00(FieldMovingActor *actor, s32 dx, s32 dz)
{
    FieldActorCollisionBounds *bounds = (FieldActorCollisionBounds *)0x801ED400;
    FieldActorCollisionMover *mover = (FieldActorCollisionMover *)0x1F800000;
    s32 x, z;
    u8 speed;

    speed = actor->speed;
    actor->x += dx * speed;
    actor->z += dz * speed;
    x = actor->x;
    z = actor->z;
    if (x >= 0 && x < (bounds->width << 8) && z >= 0 &&
        z < ((s32)(bounds->height << 16) >> 7))
    {
        mover->x = x;
        mover->y = actor->y;
        mover->z = actor->z;
        mover->dx = 0;
        mover->dy = 0;
        mover->dz = 0;
        if (((FieldObjectVisualKind *)((Blk80094508_FieldActorPartDef *)g_field_object_parts))[actor->slot].kind == 0x40)
        {
            mover->radius = 12;
            mover->mode.bits.step = 8;
        }
        else
        {
            mover->radius = 9;
            mover->mode.bits.step = 6;
        }
        mover->depth = 16;
        /* Separate bitfield clears preserve the two target mask operations. */
        mover->mode.bits.bit17 = 0;
        mover->mode.bits.bit16 = 0;
        mover->contact = ((FieldActorCollisionResult *)((FieldFollowSlot *)g_field_object_states))[actor->slot].contact;
        mover->surface = ((FieldActorCollisionResult *)((FieldFollowSlot *)g_field_object_states))[actor->slot].surface;
        func_8005B6AC(mover);
        ((FieldActorCollisionResult *)((FieldFollowSlot *)g_field_object_states))[actor->slot].contact = mover->contact;
        ((FieldActorCollisionResult *)((FieldFollowSlot *)g_field_object_states))[actor->slot].surface = mover->surface;
        ((FieldActorCollisionResult *)((FieldFollowSlot *)g_field_object_states))[actor->slot].height = mover->height / 256;
    }
    else
    {
        ((FieldActorCollisionResult *)((FieldFollowSlot *)g_field_object_states))[actor->slot].contact = -1;
        ((FieldActorCollisionResult *)((FieldFollowSlot *)g_field_object_states))[actor->slot].surface = 0;
        ((FieldActorCollisionResult *)((FieldFollowSlot *)g_field_object_states))[actor->slot].height = 0;
    }
}

/* field_actor_resource_states: Advance actor states after their resource requests complete. */

#include "common.h"

typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;   /* 0x2A */
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;    /* 0x3A */
    u8 unk3B;    /* 0x3B */
    u8 pad3C[0x178 - 0x3C];
    u8 unk178;   /* 0x178 */
    u8 pad179[0x23C - 0x179];
} ActorRec;

typedef struct {
    u8 pad0[0xE];
    u16 unkE;    /* 0x0E */
    u8 pad10[0x14 - 0x10];
} ResEntry;

/**
 * @brief Updates the actor state when its resource query succeeds.
 *
 * @return The guard/query result, or 0x8E after a successful update.
 * @note 100% match. The function returns the value already carried in v0;
 *       that return lifetime naturally preserves the target delay-slot nops.
 */
s32 func_80094EA4(ActorRec *arg0)
{
    s32 result;

    result = ((ActorRec *)g_field_object_states)[arg0->unk3A].unk178 & 1;
    if (result == 0)
    {
        result = field_start_bound_action_animation(arg0->unk3A, 0, 0, ((ResEntry *)g_field_resource_entries)[arg0->unk3B].unkE);
        if (result != 0)
        {
            result = 0x8E;
            arg0->unk2A = result;
        }
    }
    return result;
}

/**
 * @brief Updates the actor state when its resource query succeeds.
 *
 * @return The guard/query result, or 0x94 after a successful update.
 * @note 100% match. Twin of func_80094EA4 with a different success state.
 */
s32 func_80094F40(ActorRec *arg0)
{
    s32 result;

    result = ((ActorRec *)g_field_object_states)[arg0->unk3A].unk178 & 1;
    if (result == 0)
    {
        result = field_start_bound_action_animation(arg0->unk3A, 0, 0, ((ResEntry *)g_field_resource_entries)[arg0->unk3B].unkE);
        if (result != 0)
        {
            result = 0x94;
            arg0->unk2A = result;
        }
    }
    return result;
}

/* field_actor_animation_resume: Detect released animation slots and resume the actor record animation. */

/* func_80094FDC */
#include "common.h"

typedef struct
{
    u8 pad0[0x18];
    s32 unk18;
} Struct_D80105880;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x244 - 0x25];
} Blk80094FDC_FieldActorState;

typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[0x2A - 0x26];
    s16 unk2A;
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;
} Struct_D800FDF58;

extern void func_80095074(Struct_D800FDF58 *rec);

/**
 * @brief Resets a field record when its selected actor slot is free.
 *
 * The record's selector at 0x3A chooses one of three g_field_actor_bindings entries, with
 * values >= 2 clamped to the third entry. If that entry's actor slot is free,
 * clears the record timer, writes the 0xFF sentinel, and calls func_80095074.
 *
 * @note The s32 return type, despite the lack of an explicit return statement,
 *       is required to preserve the target v0 lifetime. gcc272_cdk, 100%.
 */
s32 func_80094FDC(Struct_D800FDF58 *rec)
{
    Blk80094FDC_FieldActorState *actors;
    u8 *base;
    s32 offset;
    s32 idx;
    Blk80094FDC_FieldActorState *actor;

    actors = ((Blk80094FDC_FieldActorState *)g_field_actor_slots);
    base = (u8 *)((Struct_D80105880 *)g_field_actor_bindings);
    if (rec->unk3A < 2)
        offset = rec->unk3A * 0x1C;
    else
        offset = 0x38;
    idx = *(s32 *)(base + offset + 0x18);
    actor = actors + idx;
    if (actor->unk24 == 0)
    {
        rec->unk2A = 0;
        rec->unk25 = 0xFF;
        func_80095074(rec);
    }
}

/* func_80095074 */
#include "common.h"

typedef struct
{
    u8 pad0[0x60];
    u8 unk60[4];   /* 0x60 */
    u8 pad64[0x16C - 0x64];
    u8 unk16C;     /* 0x16C */
    u8 pad16D[0x23C - 0x16D];
} Struct_D80105AE0;

extern s32 func_800839F8(s32 arg0, s32 arg1);
extern s32 func_80083EEC(s32 arg0, s32 arg1, s32 arg2);
extern void field_start_actor_animation(s32 slot_index, int target_count, u8 *targets);

/**
 * @see decomp.me (100%) TODO
 */
void func_80095074(Struct_D800FDF58 *rec)
{
    s32 i;
    s32 anim;
    s32 anim_id;

    if (((Struct_D80105AE0 *)g_field_object_states)[rec->unk3A].unk16C == 0xFF)
    {
        return;
    }
    if (((Struct_D80105AE0 *)g_field_object_states)[rec->unk3A].unk16C == 0x1F)
    {
        for (i = 0; i < 4; i++)
        {
            if (((Struct_D80105AE0 *)g_field_object_states)[rec->unk3A].unk60[i] != 0)
            {
                anim_id = ((Struct_D80105AE0 *)g_field_object_states)[rec->unk3A].unk16C;
                anim = func_800839F8(rec->unk3A, 0);
                if (anim != -1)
                {
                    if (func_80083EEC(rec->unk3A, anim, anim_id))
                    {
                        field_start_actor_animation(anim, 0, 0);
                    }
                }
                return;
            }
        }
        return;
    }
    anim_id = ((Struct_D80105AE0 *)g_field_object_states)[rec->unk3A].unk16C;
    anim = func_800839F8(rec->unk3A, 0);
    if (anim != -1)
    {
        if (func_80083EEC(rec->unk3A, anim, anim_id))
        {
            field_start_actor_animation(anim, 0, 0);
        }
    }
}

/* func_80095168 */
#include "common.h"

extern void func_80095074(Struct_D800FDF58 *rec);

/**
 * @brief Resets an actor record when its target field-actor slot is free.
 *
 * When the slot at @c g_field_actor_slots[rec->unk3A + 0x40] has a zero
 * @c unk24 (unclaimed), clears the record's @c unk2A, sets @c unk25 to the
 * sentinel 0xFF, and hands the record to func_80095074.
 *
 * @param rec Actor record to reset.
 * @return Unused status value (the return register is left live by the
 *         original, but no caller consumes it).
 */
s32 func_80095168(Struct_D800FDF58 *rec)
{
    if (g_field_actor_slots[rec->unk3A + 0x40].is_active == 0)
    {
        rec->unk2A = 0;
        rec->unk25 = 0xFF;
        func_80095074(rec);
    }
}

/* field_actor_sequence_runtime: Execute object sequences, manage their animation actors, and update tint flashing. */
#include "field_actor_runtime.h"
#include "field_contact_geometry.h"
#include "sdk/memory.h"

#define FIELD_SEQUENCE_DISPLACEMENT_SCRATCH 0x1F800000
#define FIELD_SEQUENCE_BINDING_COUNT 3
#define FIELD_SEQUENCE_SHARED_BINDING (FIELD_SEQUENCE_BINDING_COUNT - 1)
#define FIELD_SEQUENCE_ACTOR_LIMIT 80
#define FIELD_SEQUENCE_TARGET_LIMIT 14
#define FIELD_SEQUENCE_FRAME_WAIT 1
#define FIELD_SEQUENCE_NO_ACTOR 0xFF
#define FIELD_SEQUENCE_RESTORE_TEMPLATE 2
#define FIELD_SEQUENCE_MOTION_SCALE_SHIFT 6
#define FIELD_SEQUENCE_ROW_SIZE 32
#define FIELD_SEQUENCE_BANK_SIZE (24 * FIELD_SEQUENCE_ROW_SIZE)
#define FIELD_SEQUENCE_COMMAND_NONE 0xFFFF
#define FIELD_SEQUENCE_ANIMATION_OVERRIDE 0x4000
#define FIELD_SEQUENCE_TRANSIENT_ACTOR 0x8000
#define FIELD_SEQUENCE_ANIMATION_MASK 0x3FF
#define FIELD_SEQUENCE_MOVEMENT_MASK 0x1800
#define FIELD_SEQUENCE_FACING 0x80
#define FIELD_OBJECT_TINT_FLASH 0x8000
#define FIELD_TINT_BLINK_BIT 4
#define FIELD_TINT_DIM_NUMERATOR 100
#define FIELD_TINT_DIM_DENOMINATOR 128

/** @brief Control bytes following the frame values in a 32-byte sequence row. */
typedef enum
{
    FIELD_SEQUENCE_START_TARGETS_0 = 0xEB,
    FIELD_SEQUENCE_START_TARGETS_1 = 0xEC,
    FIELD_SEQUENCE_START_TARGETS_2 = 0xED,
    FIELD_SEQUENCE_START_CURRENT_TARGETS = 0xEE,
    FIELD_SEQUENCE_WAIT_REPEAT = 0xEF,
    FIELD_SEQUENCE_DELAY = 0xF0,
    FIELD_SEQUENCE_WAIT_ANIMATION = 0xF1,
    FIELD_SEQUENCE_TOGGLE_CONTROL_14 = 0xF2,
    FIELD_SEQUENCE_TOGGLE_CONTROL_15 = 0xF3,
    FIELD_SEQUENCE_TOGGLE_FACING = 0xF4,
    FIELD_SEQUENCE_START_RESOURCE = 0xF5,
    FIELD_SEQUENCE_START_0 = 0xF6,
    FIELD_SEQUENCE_START_1 = 0xF7,
    FIELD_SEQUENCE_START_2 = 0xF8,
    FIELD_SEQUENCE_START_CURRENT = 0xF9,
    FIELD_SEQUENCE_SET_ANIMATION = 0xFA,
    FIELD_SEQUENCE_ALLOCATE_0 = 0xFB,
    FIELD_SEQUENCE_ALLOCATE_1 = 0xFC,
    FIELD_SEQUENCE_ALLOCATE_2 = 0xFD,
    FIELD_SEQUENCE_ALLOCATE_CURRENT = 0xFE,
    FIELD_SEQUENCE_END = 0xFF
} FieldSequenceOpcode;

s32 func_800839F8(s32 owner_index, s32 require_idle_binding);
s32 func_80083EEC(s32 owner_index, s32 actor_index, s32 resource_index);
void func_80084424(s32 owner_index);
void func_80086494(s32 object_index);
void field_restart_actor_animation(FieldMotionRecord* object);

/**
 * @brief Consume a signed displacement remainder and apply a scaled movement step.
 * @param object Moving object; motion_divisor is the divisor and motion_remainder the remainder.
 * @param direction_x Horizontal direction scale.
 * @param vertical_step Vertical displacement before part scaling.
 * @param direction_z Depth direction scale.
 */
void field_apply_sequence_displacement(FieldMotionRecord* object, s32 direction_x, s32 vertical_step, s32 direction_z)
{
    s32 step;
    FieldActorPartDef* part;
    s32* out;

    out = (s32*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    if (object->motion_scale == 0)
    {
        object->motion_parameter = 0;
        return;
    }
    step = (s8)object->motion_remainder / object->motion_divisor;
    object->motion_remainder = object->motion_remainder - step;
    part = &g_field_object_parts[object->source_object_index];
    out[0] = (step * direction_x * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    out[1] = (vertical_step * part->footprint_scale_y) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    out[2] = (step * direction_z * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    field_resolve_actor_movement(object, out, 0);
}

/**
 * @brief Mark an owned animation as running, or release its completed binding.
 * @param object Object selecting the binding, with indices above one sharing the third entry.
 * @param release_actor Clear the completed actor's active flag before releasing its binding.
 */
void field_update_sequence_actor_binding(FieldMotionRecord* object, s32 release_actor)
{
    FieldSequenceBinding* base;
    s32 object_index;
    s32 active_owner;
    s32 finished_owner;
    s32 binding_offset;
    s32 animation_binding_offset;
    s32 owner_binding_offset;
    s32 active_binding_offset;
    s32 finished_binding_offset;
    s32 release_binding_offset;

    base = g_field_actor_bindings;
    if (object->source_object_index < 2U)
    {
        binding_offset = (object->source_object_index) * sizeof(*base);
    }
    else
    {
        binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
    }
    if (((FieldSequenceBinding*)((u8*)base + binding_offset))->state != 0)
    {
        base = g_field_actor_bindings;
        if (object->source_object_index < 2U)
        {
            animation_binding_offset = (object->source_object_index) * sizeof(*base);
        }
        else
        {
            animation_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
        }
        if (field_is_actor_animation_active(((FieldSequenceBinding*)((u8*)base + animation_binding_offset))->actor_index) != 0)
        {
            base = g_field_actor_bindings;
            if (object->source_object_index < 2U)
            {
                owner_binding_offset = (object->source_object_index) * sizeof(*base);
            }
            else
            {
                owner_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
            }
            if (((FieldSequenceBinding*)((u8*)base + owner_binding_offset))->owner_object_index == object->source_object_index)
            {
                FieldActorState* actors;
                FieldSequenceBinding* lookup;

                active_owner = ((FieldSequenceBinding*)((u8*)base + owner_binding_offset))->owner_object_index;
                actors = g_field_actor_slots;
                lookup = g_field_actor_bindings;
                if ((u32)(active_owner & 0xFF) < 2U)
                {
                    active_binding_offset = (active_owner) * sizeof(*base);
                }
                else
                {
                    active_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
                }
                {
                    u32 actor_address = (u32)actors;
                    actor_address += ((FieldSequenceBinding*)((u8*)lookup + active_binding_offset))->actor_index * sizeof(*actors);
                    ((FieldActorState*)actor_address)->sequence_active = 1;
                }
            }
        }
        else
        {
            base = g_field_actor_bindings;
            if (object->source_object_index < 2U)
            {
                finished_binding_offset = (object->source_object_index) * sizeof(*base);
            }
            else
            {
                finished_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
            }
            object_index = object->source_object_index;
            finished_owner = ((FieldSequenceBinding*)((u8*)base + finished_binding_offset))->owner_object_index;
            if (finished_owner == object_index)
            {
                if (release_actor != 0)
                {
                    FieldActorState* actors;
                    FieldSequenceBinding* lookup;

                    actors = g_field_actor_slots;
                    lookup = g_field_actor_bindings;
                    if ((u32)(finished_owner & 0xFF) < 2U)
                    {
                        release_binding_offset = (finished_owner) * sizeof(*base);
                    }
                    else
                    {
                        release_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
                    }
                    {
                        u32 actor_address = (u32)actors;
                        actor_address += ((FieldSequenceBinding*)((u8*)lookup + release_binding_offset))->actor_index * sizeof(*actors);
                        ((FieldActorState*)actor_address)->is_active = 0;
                    }
                }
                func_80084424(object->source_object_index);
            }
        }
    }
}

/**
 * @brief Consume sequence commands until a frame, wait, delay, or terminator is reached.
 * @param object Object whose runtime state holds the cursor and animation binding.
 * @param script_index Row within the player's selected sequence bank.
 * @return One if the initial cursor already points at the terminator; zero otherwise.
 * @note Animation targets are expanded to four-byte entries for the animation API.
 * @note Command cases retain independent movement-flag updates and target-copy cursors.
 */
s32 field_execute_actor_sequence(FieldMotionRecord* object, s32 script_index)
{
    FieldObjectRuntime* slots;
    FieldSequencePlayer* players;
    u8* programs;
    u8* initial_program;
    u8* initial_program_base;
    FieldSequenceBinding* bindings;
    FieldSequenceBinding* binding_test;
    FieldActorState* actors;
    s32 parameters[FIELD_SEQUENCE_TARGET_LIMIT];
    FieldActorState* copy_source;
    FieldActorState* template_actor;
    s32 actor_index;
    s32 script_offset;
    s32 bank_offset;
    s32 pending_command;
    s32 target_index;
    s32 current_target_index;
    s32 animation_command;
    u32 delay_operand;
    u32 resource_operand;
    u32 animation_operand;
    s32 clear_slot;
    s32 movement_mask;
    s32 cursor;
    s32 result;
    s32 initial_binding_offset;
    s32 restore_binding_offset;
    s32 release_binding_offset;
    s32 updated_flags;
    u8* opcode_ptr;
    s32 target_owner;
    s32 current_target_owner;
    u8 delay_owner;
    u8 resource_owner;
    u8 animation_owner;
    s32 allocation_owner;
    u8 command;
    u8 pending_owner;
    u8 initial_owner;
    u8 command_owner;
    u8 opcode;
    s32 command_slot;
    FieldObjectRuntime* target_state;
    FieldObjectRuntime* allocation_state;
    FieldObjectRuntime* animation_state;
    FieldObjectRuntime* pending_state;
    FieldObjectRuntime* current_target_state;
    FieldObjectRuntime* current_animation_state;
    FieldObjectRuntime* current_allocation_state;
    FieldActorState* pending_actor;
    FieldActorState* pending_actors;
    FieldObjectRuntime* flag_state;

    g_field_object_states[object->source_object_index].sequence_delay = 0;
    pending_owner = object->source_object_index;
    pending_state = &g_field_object_states[pending_owner];
    pending_command = pending_state->sequence_command;
    if (pending_command != FIELD_SEQUENCE_COMMAND_NONE)
    {
        if (pending_command & FIELD_SEQUENCE_TRANSIENT_ACTOR)
        {
            if (pending_state->contact.bytes.animation_actor_index < FIELD_SEQUENCE_ACTOR_LIMIT)
            {
                pending_actors = g_field_actor_slots;
                pending_actor = &pending_actors[pending_state->contact.bytes.animation_actor_index];
                if ((pending_actor->is_active != 0) && (pending_actor->owner_object_index == pending_owner))
                {
                    pending_actor->is_active = 0U;
                }
            }
            g_field_object_states[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
        }
    }
    initial_program_base = g_field_actor_sequence_data;
    initial_owner = object->source_object_index;
    cursor = g_field_object_states[initial_owner].sequence_cursor;
    initial_program = (script_index * FIELD_SEQUENCE_ROW_SIZE) + (g_field_player_records[initial_owner].sequence_bank * FIELD_SEQUENCE_BANK_SIZE) +
                      initial_program_base + cursor;
    if (*initial_program == FIELD_SEQUENCE_END)
    {
        return 1;
    }
    if (cursor == 1)
    {
        binding_test = g_field_actor_bindings;
        if (initial_owner < 2U)
        {
            initial_binding_offset = (initial_owner) * sizeof(*bindings);
        }
        else
        {
            initial_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*bindings);
        }
        result = 0;
        if (((FieldSequenceBinding*)((u8*)binding_test + initial_binding_offset))->state != FIELD_SEQUENCE_RESTORE_TEMPLATE)
        {
            return 0;
        }
        {
            FieldActorState* restore_actors;
            FieldSequenceBinding* restore_bindings;
            restore_actors = g_field_actor_slots;
            restore_actors[g_field_actor_bindings[object->source_object_index].actor_index].sequence_active = 0;
            restore_actors[g_field_actor_bindings[object->source_object_index].actor_index].track_count = 0;
            restore_bindings = g_field_actor_bindings;
            if (object->source_object_index < 2U)
            {
                restore_binding_offset = (object->source_object_index) * sizeof(*restore_bindings);
            }
            else
            {
                restore_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*restore_bindings);
            }
            copy_source = &g_field_actor_slots[((FieldSequenceBinding*)((u8*)restore_bindings + restore_binding_offset))->actor_index];
            if (object->source_object_index < 2U)
            {
                template_actor = &g_field_actor_templates[object->source_object_index].actor;
            }
            else
            {
                template_actor = &g_field_shared_actor_template;
            }
            /* Restore the bound runtime actor into its player template. */
            bcopy((const u8*)copy_source, (u8*)template_actor, sizeof(*copy_source));
            actors = g_field_actor_slots;
            bindings = g_field_actor_bindings;
            if (object->source_object_index < 2U)
            {
                release_binding_offset = object->source_object_index * sizeof(*bindings);
            }
            else
            {
                release_binding_offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(*bindings);
            }
        }
        {
            s32 actor_offset = ((FieldSequenceBinding*)((u8*)bindings + release_binding_offset))->actor_index * sizeof(*actors);
            ((FieldActorState*)((s32)actors + actor_offset))->is_active = 0;
        }
    }
    programs = g_field_actor_sequence_data;
    players = g_field_player_records;
    slots = g_field_object_states;
    script_offset = script_index * FIELD_SEQUENCE_ROW_SIZE;
    command_slot = object->source_object_index;
    opcode_ptr = script_offset + players[command_slot].sequence_bank * FIELD_SEQUENCE_BANK_SIZE + programs + cursor;
    opcode = *opcode_ptr;
    result = 0;
    /* Frame bytes stop dispatch; command bytes may consume additional operands. */
    for (; opcode >= FIELD_SEQUENCE_START_TARGETS_0;
         command_slot = object->source_object_index,
         bank_offset = script_offset + players[command_slot].sequence_bank * FIELD_SEQUENCE_BANK_SIZE,
         opcode_ptr = (u8*)(bank_offset + (s32)programs + cursor),
         opcode = *opcode_ptr)
    {
        switch (opcode)
        {
        case FIELD_SEQUENCE_END:
            g_field_object_states[command_slot].sequence_cursor = cursor;
            object->motion_scale = FIELD_SEQUENCE_FRAME_WAIT;
            object->saved_state = 0;
            object->animation_active = 1;
            object->facing_or_reward_kind = (u8)(object->facing_or_reward_kind & FIELD_SEQUENCE_FACING);
            return 0;
        default:
            command = *opcode_ptr;
            switch (command)
            {
            case FIELD_SEQUENCE_START_TARGETS_0:
            case FIELD_SEQUENCE_START_TARGETS_1:
            case FIELD_SEQUENCE_START_TARGETS_2:
                {
                    FieldObjectRuntime* source_state;
                    s32 kind_flags;
                    s32 sequence_command;
                    source_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    source_state = (FieldObjectRuntime*)((s32)source_state + (u8*)slots);
                    kind_flags = ((command - FIELD_SEQUENCE_START_TARGETS_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                    sequence_command = source_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                    target_state = source_state;
                    sequence_command |= kind_flags;
                    sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    target_state->sequence_command = sequence_command;
                }
                target_owner = object->source_object_index;
                actor_index = field_allocate_sequence_actor(target_owner, slots[target_owner].sequence_command);
                {
                    s32 copy_owner;
                    s32* target_output;
                    copy_owner = object->source_object_index;
                    target_index = 0;
                    if (slots[copy_owner].contact.bytes.target_count != 0)
                    {
                        target_output = parameters;
                        do
                        {
                            *target_output = slots[copy_owner].targets[target_index];
                            target_index++;
                            target_output++;
                            copy_owner = object->source_object_index;
                        } while (target_index < slots[copy_owner].contact.bytes.target_count);
                    }
                }
                field_start_actor_animation(actor_index, slots[object->source_object_index].contact.bytes.target_count, (u8*)parameters);
                cursor += 1;
                slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                continue;
            case FIELD_SEQUENCE_START_CURRENT_TARGETS:
                current_target_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                current_target_state = (FieldObjectRuntime*)((s32)current_target_state + (u8*)slots);
                current_target_state->sequence_command = current_target_state->current_sequence_animation;
                current_target_owner = object->source_object_index;
                actor_index = field_allocate_sequence_actor(current_target_owner, slots[current_target_owner].current_sequence_animation);
                slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                {
                    s32 copy_owner;
                    s32* target_output;
                    copy_owner = object->source_object_index;
                    current_target_index = 0;
                    if (slots[copy_owner].contact.bytes.target_count != 0)
                    {
                        target_output = parameters;
                        do
                        {
                            *target_output = slots[copy_owner].targets[current_target_index];
                            current_target_index++;
                            target_output++;
                            copy_owner = object->source_object_index;
                        } while (current_target_index < slots[copy_owner].contact.bytes.target_count);
                    }
                }
                field_start_actor_animation(actor_index, slots[object->source_object_index].contact.bytes.target_count, (u8*)parameters);
                cursor += 1;
                continue;
            case FIELD_SEQUENCE_DELAY:
                result = 0;
                delay_owner = object->source_object_index;
                pending_state = (FieldObjectRuntime*)(delay_owner * sizeof(*slots));
                delay_operand = script_offset + players[delay_owner].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
                delay_operand += (u32)programs;
                delay_operand += cursor;
                pending_state = (FieldObjectRuntime*)((u8*)pending_state + (s32)slots);
                pending_state->sequence_delay = ((u8*)delay_operand)[1];
                cursor += 2;
                slots[object->source_object_index].sequence_cursor = cursor;
                return result;
            case FIELD_SEQUENCE_WAIT_REPEAT:
            case FIELD_SEQUENCE_WAIT_ANIMATION:
                slots[object->source_object_index].sequence_cursor = cursor;
                return 0;
            case FIELD_SEQUENCE_TOGGLE_CONTROL_14:
                flag_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                flag_state = (FieldObjectRuntime*)((s32)flag_state + (u8*)slots);
                updated_flags = flag_state->object_flags ^ 0x4000;
                flag_state->object_flags = updated_flags;
                cursor += 1;
                func_80086494(object->source_object_index);
                continue;
            case FIELD_SEQUENCE_TOGGLE_CONTROL_15:
                flag_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                flag_state = (FieldObjectRuntime*)((s32)flag_state + (u8*)slots);
                updated_flags = flag_state->object_flags ^ 0x8000;
                flag_state->object_flags = updated_flags;
                cursor += 1;
                func_80086494(object->source_object_index);
                continue;
            case FIELD_SEQUENCE_TOGGLE_FACING:
                cursor += 1;
                object->facing_or_reward_kind = (u8)(object->facing_or_reward_kind ^ FIELD_SEQUENCE_FACING);
                continue;
            case FIELD_SEQUENCE_START_RESOURCE:
                actor_index = func_800839F8(object->source_object_index, 0);
                if (actor_index != -1)
                {
                    resource_owner = object->source_object_index;
                    resource_operand = script_offset + players[resource_owner].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
                    resource_operand += (u32)programs;
                    resource_operand += cursor;
                    func_80083EEC(resource_owner, actor_index, ((u8*)resource_operand)[1]);
                    field_start_actor_animation(actor_index, 0U, NULL);
                }
                cursor += 2;
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state;
                    cleared_state = (FieldObjectRuntime*)(clear_slot * (s32)sizeof(*slots) + (s32)slots);
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_START_0:
            case FIELD_SEQUENCE_START_1:
            case FIELD_SEQUENCE_START_2:
                animation_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                animation_state = (FieldObjectRuntime*)((s32)animation_state + (u8*)slots);
                {
                    s32 kind_flags;
                    s32 sequence_command;
                    kind_flags = ((command - FIELD_SEQUENCE_START_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                    sequence_command = animation_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                    sequence_command |= kind_flags;
                    sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    animation_state->sequence_command = sequence_command;
                }
                animation_owner = object->source_object_index;
                field_start_actor_animation(field_allocate_sequence_actor(animation_owner, slots[animation_owner].sequence_command), 0U, NULL);
                slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state;
                    cleared_state = (FieldObjectRuntime*)(clear_slot * (s32)sizeof(*slots) + (s32)slots);
                    cursor += 1;
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_START_CURRENT:
                current_animation_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                current_animation_state = (FieldObjectRuntime*)((s32)current_animation_state + (u8*)slots);
                current_animation_state->sequence_command = current_animation_state->current_sequence_animation;
                allocation_owner = object->source_object_index;
                actor_index = field_allocate_sequence_actor(allocation_owner, slots[allocation_owner].current_sequence_animation);
                slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                field_start_actor_animation(actor_index, 0U, NULL);
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state;
                    cleared_state = (FieldObjectRuntime*)(clear_slot * (s32)sizeof(*slots) + (s32)slots);
                    cursor += 1;
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_SET_ANIMATION:
                command_owner = object->source_object_index;
                target_state = (FieldObjectRuntime*)(command_owner * sizeof(*slots));
                animation_operand = script_offset + players[command_owner].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
                animation_operand += (u32)programs;
                animation_operand += cursor;
                target_state = (FieldObjectRuntime*)((s32)target_state + (s32)slots);
                target_state->sequence_command = ((u8*)animation_operand)[1];
                cursor += 2;
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state;
                    cleared_state = (FieldObjectRuntime*)(clear_slot * (s32)sizeof(*slots) + (s32)slots);
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_ALLOCATE_0:
            case FIELD_SEQUENCE_ALLOCATE_1:
            case FIELD_SEQUENCE_ALLOCATE_2:
                allocation_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                allocation_state = (FieldObjectRuntime*)((s32)allocation_state + (u8*)slots);
                {
                    s32 kind_flags;
                    s32 sequence_command;
                    kind_flags = ((command - FIELD_SEQUENCE_ALLOCATE_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                    sequence_command = allocation_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                    sequence_command |= kind_flags;
                    sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    allocation_state->sequence_command = sequence_command;
                }
                allocation_owner = object->source_object_index;
                animation_command = slots[allocation_owner].sequence_command;
                field_allocate_sequence_actor(allocation_owner, animation_command);
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state;
                    cleared_state = (FieldObjectRuntime*)(clear_slot * (s32)sizeof(*slots) + (s32)slots);
                    cursor += 1;
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_ALLOCATE_CURRENT:
                current_allocation_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                current_allocation_state = (FieldObjectRuntime*)((s32)current_allocation_state + (u8*)slots);
                current_allocation_state->sequence_command = current_allocation_state->current_sequence_animation;
                allocation_owner = object->source_object_index;
                animation_command = slots[allocation_owner].current_sequence_animation;
                field_allocate_sequence_actor(allocation_owner, animation_command);
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state;
                    cleared_state = (FieldObjectRuntime*)(clear_slot * (s32)sizeof(*slots) + (s32)slots);
                    cursor += 1;
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            default:
                continue;
            }
        }
    }
    {
        u8* frame_programs;
        FieldSequencePlayer* frame_players;
        FieldObjectRuntime* frame_slots;
        u32 frame_address;
        s32 frame_offset;
        frame_programs = g_field_actor_sequence_data;
        frame_players = g_field_player_records;
        frame_offset = (script_index * FIELD_SEQUENCE_ROW_SIZE) + frame_players[object->source_object_index].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
        frame_address = frame_offset;
        frame_address += (u32)frame_programs;
        frame_address += cursor;
        cursor++;
        frame_slots = g_field_object_states;
        object->facing_or_reward_kind = *(u8*)frame_address + (object->facing_or_reward_kind & FIELD_SEQUENCE_FACING);
        frame_slots[object->source_object_index].sequence_cursor = cursor;
        object->motion_scale = FIELD_SEQUENCE_FRAME_WAIT;
        object->saved_state = 0;
        object->animation_active = 1;
        return 0;
    }
}

/**
 * @brief Copy an object template into a free actor slot and select its animation.
 * @param index Object/template index, clamped to two only for the binding table.
 * @param flags Animation override flag and two-bit animation index.
 * @return Assigned actor slot, or -1 when allocation fails.
 */
s32 field_allocate_sequence_actor(s32 index, s32 flags)
{
    s32 slot, binding;
    FieldActorState *actor, *updated, *slots;
    FieldSequenceBinding* bindings;
    slot = func_800839F8(index, 0);
    if (slot != -1)
    {
        actor = &g_field_actor_slots[slot];
        bcopy((const u8*)&g_field_actor_templates[index], (u8*)actor, sizeof(*actor));
        actor->is_active = 1;
        actor->actor_index = slot;
        if (flags & FIELD_SEQUENCE_ANIMATION_OVERRIDE)
        {
            actor->animation_index = (flags >> 12) & 3;
            actor->unknown_0x222 = actor->animations[actor->animation_index].unknown_0x12;
        }
        else
        {
            actor->animation_index = 0;
            actor->unknown_0x222 = actor->animations->unknown_0x12;
        }
        slots = g_field_actor_slots;
        updated = &slots[slot];
        binding = index;
        updated->animation_mode = updated->animations[updated->animation_index].animation_mode;
        updated->animation = &updated->animations[updated->animation_index];
        g_field_object_states[binding].contact.bytes.animation_actor_index = slot;
        bindings = g_field_actor_bindings;
        if (binding >= FIELD_SEQUENCE_BINDING_COUNT)
        {
            binding = FIELD_SEQUENCE_SHARED_BINDING;
        }
        bindings[binding].actor_index = slot;
    }
    else
    {
        g_field_object_states[index].contact.bytes.animation_actor_index = FIELD_SEQUENCE_NO_ACTOR;
    }
    return slot;
}

/**
 * @brief Reset sequence frame progress and restart the object's animation.
 * @param object Object whose movement flags and animation state are reset.
 */
void field_restart_sequence_animation(FieldMotionRecord* object)
{
    object->motion_scale = FIELD_SEQUENCE_FRAME_WAIT;
    object->saved_state = 0;
    object->animation_active = 1;

    g_field_object_states[object->source_object_index].movement.word &= ~FIELD_SEQUENCE_MOVEMENT_MASK;

    field_restart_actor_animation(object);
}

/**
 * @brief Update object tint colors for timed flashes and active selection blinking.
 * @note Flash phases dim each base color to 100/128 of its value.
 */
void field_update_object_tints(void)
{
    s32 i;
    FieldActorPartDef* visual = g_field_object_parts;
    FieldObjectRuntime* slot = g_field_object_states;
    s32 selected;
    u32 flags;
    u32 options;
    u8 timer;

    for (i = 0; i < FIELD_OBJECT_COUNT; i++)
    {
        slot = &g_field_object_states[i];
        visual = &g_field_object_parts[i];
        if (g_field_actors[i].state != 0xFF)
        {
            timer = slot->tint_flash_timer;
            if (timer != 0)
            {
                if (timer & FIELD_TINT_BLINK_BIT)
                {
                    visual->red_or_track = (slot->tint_red * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                    visual->green_or_track = (slot->tint_green * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                    visual->blue_or_track = (slot->tint_blue * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                }
                else
                {
                    visual->red_or_track = slot->tint_red;
                    visual->green_or_track = slot->tint_green;
                    visual->blue_or_track = slot->tint_blue;
                }
                timer = slot->tint_flash_timer - 1;
                slot->tint_flash_timer = timer;
                if (timer == 0)
                {
                    slot->movement.word &= ~FIELD_OBJECT_TINT_FLASH;
                }
            }
            else
            {
                if (slot->current_hp != 0 && !(slot->contact.bytes.flags_low & 1))
                {
                    flags = slot->contact.flags;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1))
                    {
                        if (!((flags >> 6) & 1))
                        {
                            selected = i;
                            if (i >= FIELD_SEQUENCE_BINDING_COUNT)
                            {
                                selected = FIELD_SEQUENCE_SHARED_BINDING;
                            }
                            if (g_field_actor_bindings[selected].owner_object_index == i)
                            {
                                selected = i;
                                if (i >= FIELD_SEQUENCE_BINDING_COUNT)
                                {
                                    selected = FIELD_SEQUENCE_SHARED_BINDING;
                                }
                                if (g_field_actor_bindings[selected].state != 0)
                                {
                                    goto check_blink;
                                }
                            }
                        }
                        slot->movement.word &= ~FIELD_OBJECT_TINT_FLASH;
                        visual->red_or_track = slot->tint_red;
                        goto restore_green;
                    }
                }
            check_blink:
                options = slot->movement.word & ~FIELD_OBJECT_TINT_FLASH;
                slot->movement.word = options;
                if (g_field_actors[i].state != 0xFF && slot->current_hp != 0 && !(slot->contact.bytes.flags_low & 1))
                {
                    flags = slot->contact.flags;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1) && g_field_active_group != 0)
                    {
                        slot->movement.word = options | FIELD_OBJECT_TINT_FLASH;
                        if (g_frame_counter & FIELD_TINT_BLINK_BIT)
                        {
                            g_field_object_parts[i].red_or_track = (slot->tint_red * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                            g_field_object_parts[i].green_or_track = (slot->tint_green * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                            g_field_object_parts[i].blue_or_track = (slot->tint_blue * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                        }
                        else
                        {
                            visual->red_or_track = slot->tint_red;
                        restore_green:
                            visual->green_or_track = slot->tint_green;
                            visual->blue_or_track = slot->tint_blue;
                        }
                    }
                }
            }
        }
    }
}
