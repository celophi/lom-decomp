/** @file field_actor_sequence_runtime.c
 * @brief Execute actor sequence commands and update motion, animation slots, templates, and colors.
 */

/* func_800951CC */
#include "common.h"

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
} FieldActorState;

typedef struct
{
    u8 pad0[0x2E];
    u8 unk2E;
    u8 pad2F[0x33 - 0x2F];
    u8 unk33;
    u8 pad34[0x48 - 0x34];
} FieldActorPartDef;

extern FieldActorPartDef D_800FE3A0[];

/**
 * @brief Update a scaled actor vector and submit it through the field transform helper.
 * @param arg0 Actor state to update.
 * @param arg1 Horizontal scale input.
 * @param arg2 Secondary scale input.
 * @param arg3 Depth scale input.
 */
void func_800951CC(FieldActorState *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_lo;
    FieldActorPartDef *part;
    s32 *out;

    out = (s32 *)0x1F800000;
    if (arg0->unk2E == 0)
    {
        arg0->unk2A = 0;
        return;
    }
    temp_lo = (s8)arg0->unk36 / arg0->unk16;
    arg0->unk36 = (u8)arg0->unk36 - temp_lo;
    part = &D_800FE3A0[arg0->unk3A];
    out[0] = (temp_lo * arg1 * part->unk2E) >> 6;
    out[1] = (arg2 * part->unk33) >> 6;
    out[2] = (temp_lo * arg3 * part->unk2E) >> 6;
    func_80097FA0(arg0, out, 0);
}


/* func_800952DC */
#include "common.h"

/** @brief Field entry identifying the actor slot to update. */
typedef struct
{
    u8 pad[0x3A];
    u8 unk3A;
} Entry;
/** @brief Actor slot with activation and animation completion bytes. */
typedef struct
{
    u8 pad[0x24];
    u8 unk24;
    u8 pad25[5];
    u8 unk2A;
    u8 pad2B[0x244 - 0x2B];
} Actor;
extern u8 D_80105880[];
extern Actor g_field_actor_slots[];
extern s32 field_is_actor_animation_active(s32);
extern void func_80084424(u8);
/**
 * @brief Mark an active animation or release its associated actor slot.
 * @param arg0 Field entry whose actor slot is checked.
 * @param arg1 Whether to clear the slot activation byte before releasing it.
 */
void func_800952DC(Entry *arg0, s32 arg1)
{
    u8 *base;
    s32 slot;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;

    base = D_80105880;
    if ((u8)arg0->unk3A < 2U)
    {
        var_v0 = arg0->unk3A * 0x1C;
    }
    else
    {
        var_v0 = 0x38;
    }
    if (*(s32 *)(base + var_v0) != 0)
    {
        base = D_80105880;
        if ((u8)arg0->unk3A < 2U)
        {
            var_v0_2 = arg0->unk3A * 0x1C;
        }
        else
        {
            var_v0_2 = 0x38;
        }
        if (field_is_actor_animation_active(*(s32 *)(base + var_v0_2 + 0x18)) != 0)
        {
            base = D_80105880;
            if ((u8)arg0->unk3A < 2U)
            {
                var_v0_3 = arg0->unk3A * 0x1C;
            }
            else
            {
                var_v0_3 = 0x38;
            }
            if (*(s32 *)(base + var_v0_3 + 0xC) == arg0->unk3A)
            {
                Actor *actors;
                u8 *lookup;

                temp_a0 = *(s32 *)(base + var_v0_3 + 0xC);
                actors = g_field_actor_slots;
                lookup = D_80105880;
                if ((u32)(temp_a0 & 0xFF) < 2U)
                {
                    var_v0_4 = temp_a0 * 0x1C;
                }
                else
                {
                    var_v0_4 = 0x38;
                }
                {
                    u32 actor_address;
                    actor_address = (u32)actors;
                    actor_address += *(s32 *)(lookup + var_v0_4 + 0x18) * 0x244;
                    ((Actor *)actor_address)->unk2A = 1;
                }
            }
        }
        else
        {
            base = D_80105880;
            if ((u8)arg0->unk3A < 2U)
            {
                var_v0_5 = arg0->unk3A * 0x1C;
            }
            else
            {
                var_v0_5 = 0x38;
            }
            slot = arg0->unk3A;
            temp_a0_2 = *(s32 *)(base + var_v0_5 + 0xC);
            if (temp_a0_2 == slot)
            {
                if (arg1 != 0)
                {
                    Actor *actors;
                    u8 *lookup;

                    actors = g_field_actor_slots;
                    lookup = D_80105880;
                    if ((u32)(temp_a0_2 & 0xFF) < 2U)
                    {
                        var_v0_6 = temp_a0_2 * 0x1C;
                    }
                    else
                    {
                        var_v0_6 = 0x38;
                    }
                    {
                        u32 actor_address;
                        actor_address = (u32)actors;
                        actor_address += *(s32 *)(lookup + var_v0_6 + 0x18) * 0x244;
                        ((Actor *)actor_address)->unk24 = 0;
                    }
                }
                func_80084424(arg0->unk3A);
            }
        }
    }
}


/* func_800954F0 */
#include "common.h"

/** @brief Per-object animation command state, with its original 0x23C-byte stride. */
typedef struct
{
    u8 pad0[0xC];
    s32 flags;
    u8 pad10[0x2C];
    s32 command, current, cursor;
    u8 pad48[0x129];
    u8 delay;
    u8 pad172[2];
    s32 state;
    u8 pad178;
    u8 actor;
    u8 pad17a;
    u8 count;
    u8 pad17c[4];
    u8 arguments[0xBC];
} ScriptSlot;
/** @brief Program-bank selector in a 0x268-byte player record. */
typedef struct
{
    u8 flags, program;
    u8 tail[0x266];
} ScriptPlayer;
/** @brief Actor backup binding, with its original 0x1C-byte stride. */
typedef struct
{
    s32 state;
    u8 pad4[0x14];
    s32 actor;
} ScriptBinding;
/** @brief Runtime actor fields read or cleared by the command handler. */
typedef struct
{
    u8 pad0[0x24];
    u8 active;
    u8 pad25[5];
    u8 mode;
    u8 pad2b[0x1FD];
    u8 owner;
    u8 pad229[9];
    u8 count;
    u8 tail[0x11];
} ScriptActor;
/** @brief Object state and animation-slot index in a 0x54-byte record. */
typedef struct
{
    u8 pad0[0x21];
    u8 state;
    u8 pad22[2];
    u8 active;
    u8 pad25[2];
    u8 progress;
    u8 pad28[6];
    u16 delay;
    u8 pad30[10];
    u8 slot;
    u8 tail[0x19];
} ScriptObject;

#define ACCESS_U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define ACCESS_U16(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define ACCESS_S32(p, o) (*(s32 *)((u8 *)(p) + (o)))
extern void bcopy(void *, void *, s32);
extern void field_start_actor_animation(s32, s32, u8 *);
extern s32 func_800839F8(s32, s32);
extern s32 func_80083EEC(s32, s32, s32);
extern void func_80086494(s32);
extern s32 func_8009615C(s32, s32);
extern ScriptPlayer D_800FD818[];
extern u8 D_800FD81C[];
extern u8 D_800FDCEC[];

extern ScriptSlot D_80105AE0[];
extern u8 D_8010AED0[];


/**
 * @brief Consume animation control commands and apply the next object frame.
 * @param object Object whose slot selects the command state and actor binding.
 * @param script_index Script row within the selected player program bank.
 * @return One if the initial cursor already points at 0xFF; zero otherwise.
 * @note Commands 0xEB through 0xFE manage actor animations, flags, and waiting.
 * @note Animation target bytes are expanded to four-byte entries on the stack.
 */
s32 func_800954F0(ScriptObject *object, s32 script_index)
{
    ScriptSlot *slots;
    ScriptPlayer *players;
    u8 *programs;
    u8 *initial_program;
    u8 *initial_program_base;
    ScriptBinding *bindings;
    ScriptBinding *binding_test;
    ScriptActor *actors;
    s32 parameters[14];
    void *copy_source;
    u8 *var_a1;
    s32 *var_a1_2;
    s32 *var_a1_3;
    s32 temp_s0;
    s32 script_offset;
    s32 bank_offset;
    s32 temp_v1;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a1_4;
    s32 operand_address_F0;
    s32 operand_address_F5;
    s32 operand_address_FA;
    s32 operand_address;
    ScriptSlot *operand_slot;
    s32 clear_slot;
    s32 cursor;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    u8 *opcode_ptr;
    u8 temp_a0_2;
    u8 temp_a0_3;
    u8 temp_a0_4;
    u8 temp_a0_5;
    u8 temp_a0_7;
    s32 temp_a0_8;
    u8 command;
    u8 temp_a2;
    u8 temp_a2_2;
    u8 temp_v1_3;
    u8 opcode;
    s32 command_slot;
    ScriptSlot *temp_a0;
    ScriptSlot *temp_a0_10;
    ScriptSlot *temp_a0_6;
    ScriptSlot *temp_a1;
    ScriptSlot *temp_v0;
    ScriptSlot *temp_v0_4;
    ScriptSlot *temp_v0_5;
    ScriptSlot *temp_v0_6;
    ScriptActor *temp_v1_2;
    u8 *actor_base;
    ScriptSlot *var_v1_2;

    D_80105AE0[object->slot].delay = 0;
    temp_a2 = object->slot;
    temp_a1 = (u8 *)&D_80105AE0[temp_a2];
    temp_v1 = temp_a1->command;
    if (temp_v1 != 0xFFFF)
    {
        if (temp_v1 & 0x8000)
        {
            if ((u8) temp_a1->actor < 0x50U)
            {
                actor_base = (u8 *)((ScriptActor *)g_field_actor_slots);
                temp_v1_2 = (temp_a1->actor * 0x244) + actor_base;
                if ((temp_v1_2->active != 0) && (temp_v1_2->owner == temp_a2))
                {
                    temp_v1_2->active = 0U;
                }
            }
            D_80105AE0[object->slot].command = 0xFFFF;
        }
    }
    initial_program_base = D_8010AED0;
    temp_a2_2 = object->slot;
    cursor = D_80105AE0[temp_a2_2].cursor;
    initial_program = (script_index << 5) + (D_800FD818[temp_a2_2].program * 0x300) + initial_program_base + cursor;
    var_v0 = 1;
    if (*initial_program != 0xFF)
    {
        if (cursor == 1)
        {
            binding_test = ((ScriptBinding *)D_80105880);
            if (temp_a2_2 < 2U)
            {
                var_v0_2 = temp_a2_2 * 0x1C;
            }
            else
            {
                var_v0_2 = 0x38;
            }
            var_v0 = 0;
            if (((ScriptBinding *)((u8 *)binding_test + var_v0_2))->state == 2)
            {
                actors = ((ScriptActor *)g_field_actor_slots);
                bindings = ((ScriptBinding *)D_80105880);
                ((ScriptActor *)(((((ScriptBinding *)(((u8 *)&bindings[object->slot])))->actor * 0x244) + (u8 *)actors)))->mode = 0;
                ((ScriptActor *)(((((ScriptBinding *)(((u8 *)&bindings[object->slot])))->actor * 0x244) + (u8 *)actors)))->count = 0;
                if ((u8) object->slot < 2U)
                {
                    var_v0_3 = object->slot * 0x1C;
                }
                else
                {
                    var_v0_3 = 0x38;
                }
                copy_source = (((ScriptBinding *)(((u8 *)bindings + var_v0_3)))->actor * 0x244) + (u8 *)actors;
                if ((u8) object->slot < 2U)
                {
                    var_a1 = (object->slot * 0x268) + D_800FD81C;
                }
                else
                {
                    var_a1 = D_800FDCEC;
                }
                /* Restore the bound runtime actor into its player template. */
                bcopy(copy_source, var_a1, 0x244);
                actors = ((ScriptActor *)g_field_actor_slots);
                bindings = ((ScriptBinding *)D_80105880);
                if ((u8) object->slot < 2U)
                {
                    var_v0_4 = object->slot * 0x1C;
                    goto release_restored_actor;
                }
                goto fallback_binding;

            }
            /* Duplicate return node #61. Try simplifying control flow for better match */
            return 0;
        }
        goto begin_commands;
sequence_finished:

        D_80105AE0[command_slot].cursor = cursor;
        object->delay = 1;
        object->progress = 0;
        object->active = 1;
        object->state = (u8) (object->state & 0x80);
        return 0;

fallback_binding:
        var_v0_4 = 0x38;
release_restored_actor:
        ((ScriptActor *)(((u8 *)actors + (((ScriptBinding *)(((u8 *)bindings + var_v0_4)))->actor * 0x244))))->active = 0;
begin_commands:
        programs = D_8010AED0;
        players = D_800FD818;
        slots = D_80105AE0;
        script_offset = script_index << 5;
        command_slot = object->slot;
        opcode_ptr = script_offset + players[command_slot].program * 0x300 + programs + cursor;
        opcode = *opcode_ptr;
        var_v0 = 0;
        if (opcode >= 0xEBU)
        {
dispatch_command:
            if (opcode != 0xFF)
            {
                command = *opcode_ptr;
                /* Frame bytes are below 0xEB; 0xFF ends the sequence. */
                switch (command)
                {
                    case 0xEB:
                    case 0xEC:
                    case 0xED:
                    temp_a0 = (ScriptSlot *)(object->slot * 0x23C);
                    temp_a0 = (ScriptSlot *)((s32)temp_a0 + (u8 *)slots);
                    do
                    {
                        s32 kind;
                        kind = ((command - 0xEB) << 12) | 0x8000;
                        temp_a0->command = kind | (temp_a0->current & 0x3FF) | 0x4000;
                    } while (0);
                    temp_a0_2 = object->slot;
                    temp_s0 = func_8009615C(temp_a0_2, slots[temp_a0_2].command);
                    for (var_a0_2 = 0; var_a0_2 < slots[object->slot].count; var_a0_2++)
                    {
                        parameters[var_a0_2] = slots[object->slot].arguments[var_a0_2];
                    }                    field_start_actor_animation(temp_s0, slots[object->slot].count, (u8 *)parameters);
                    cursor += 1;
                    slots[object->slot].command = 0xFFFF;
                    goto next_command;
                    case 0xEE:
                    temp_v0 = (ScriptSlot *)(object->slot * 0x23C);
                    temp_v0 = (ScriptSlot *)((s32)temp_v0 + (u8 *)slots);
                    temp_v0->command = (s32) temp_v0->current;
                    temp_a0_3 = object->slot;
                    temp_s0 = func_8009615C(temp_a0_3, slots[temp_a0_3].current);
                    slots[object->slot].command = 0xFFFF;
                    for (var_a0_3 = 0; var_a0_3 < slots[object->slot].count; var_a0_3++)
                    {
                        parameters[var_a0_3] = slots[object->slot].arguments[var_a0_3];
                    }                    cursor += 1;
                    field_start_actor_animation(temp_s0, slots[object->slot].count, (u8 *)parameters);
                    goto next_command;
                    case 0xF0:
                    var_v0 = 0;
                    temp_a0_4 = object->slot;
                    operand_slot = (ScriptSlot *)(temp_a0_4 * 0x23C);
                    operand_address_F0 = script_offset + players[temp_a0_4].program * 0x300;
                    operand_address_F0 += (s32)programs;
                    operand_address_F0 += cursor;
                    operand_slot = (ScriptSlot *)((u8 *)operand_slot + (s32)slots);
                    operand_slot->delay = (u8) *(u8 *)(operand_address_F0 + 1);
                    cursor += 2;
                    slots[object->slot].cursor = cursor;
                    return var_v0;
                    case 0xEF:
                    case 0xF1:
                    goto save_cursor;
save_cursor:
                    slots[object->slot].cursor = cursor;
                    return 0;
                    case 0xF2:
                    var_v1_2 = (ScriptSlot *)(object->slot * 0x23C);
                    var_v1_2 = (ScriptSlot *)((s32)var_v1_2 + (u8 *)slots);
                    var_v0_5 = var_v1_2->flags ^ 0x4000;
                    goto refresh_actor_flags;
                    case 0xF3:
                    var_v1_2 = (ScriptSlot *)(object->slot * 0x23C);
                    var_v1_2 = (ScriptSlot *)((s32)var_v1_2 + (u8 *)slots);
                    var_v0_5 = var_v1_2->flags ^ 0x8000;
                    goto refresh_actor_flags;
refresh_actor_flags:
                    var_v1_2->flags = var_v0_5;
                    cursor += 1;
                    func_80086494(object->slot);
                    goto next_command;
                    case 0xF4:
                    cursor += 1;
                    object->state = (u8) (object->state ^ 0x80);
                    goto next_command;
                    case 0xF5:
                    temp_s0 = func_800839F8(object->slot, 0);
                    if (temp_s0 != -1)
                    {
                        temp_a0_5 = object->slot;
                    operand_address_F5 = script_offset + players[temp_a0_5].program * 0x300;
                    operand_address_F5 += (s32)programs;
                    operand_address_F5 += cursor;
                        func_80083EEC(temp_a0_5, temp_s0, *(u8 *)(operand_address_F5 + 1));
                        field_start_actor_animation(temp_s0, 0U, NULL);
                    }
                    cursor += 2;
                    clear_slot = object->slot;
                    goto clear_actor_state;
                    case 0xF6:
                    case 0xF7:
                    case 0xF8:
                    temp_a0_6 = (ScriptSlot *)(object->slot * 0x23C);
                    temp_a0_6 = (ScriptSlot *)((s32)temp_a0_6 + (u8 *)slots);
                    do
                    {
                        s32 kind;
                        kind = ((command - 0xF6) << 12) | 0x8000;
                        temp_a0_6->command = kind | (temp_a0_6->current & 0x3FF) | 0x4000;
                    } while (0);
                    temp_a0_7 = object->slot;
                    field_start_actor_animation(func_8009615C(temp_a0_7, slots[temp_a0_7].command), 0U, NULL);
                    slots[object->slot].command = 0xFFFF;
                    goto advance_actor_command;
                    case 0xF9:
                    temp_v0_5 = (ScriptSlot *)(object->slot * 0x23C);
                    temp_v0_5 = (ScriptSlot *)((s32)temp_v0_5 + (u8 *)slots);
                    temp_v0_5->command = (s32) temp_v0_5->current;
                    temp_a0_8 = object->slot;
                    temp_s0 = func_8009615C(temp_a0_8, slots[temp_a0_8].current);
                    slots[object->slot].command = 0xFFFF;
                    field_start_actor_animation(temp_s0, 0U, NULL);
                    goto advance_actor_command;
                    case 0xFA:
                    temp_v1_3 = object->slot;
                    operand_slot = &slots[temp_v1_3];
                    operand_address_FA = script_offset + players[temp_v1_3].program * 0x300;
                    operand_address_FA += (s32)programs;
                    operand_address_FA += cursor;
                    operand_slot->command = (s32) *(u8 *)(operand_address_FA + 1);
                    cursor += 2;
                    clear_slot = object->slot;
                    goto clear_actor_state;
                    case 0xFB:
                    case 0xFC:
                    case 0xFD:
                    temp_a0_10 = (ScriptSlot *)(object->slot * 0x23C);
                    temp_a0_10 = (ScriptSlot *)((s32)temp_a0_10 + (u8 *)slots);
                    do
                    {
                        s32 kind;
                        kind = ((command - 0xFB) << 12) | 0x8000;
                        temp_a0_10->command = kind | (temp_a0_10->current & 0x3FF) | 0x4000;
                    } while (0);
                    temp_a0_8 = object->slot;
                    var_a1_4 = slots[temp_a0_8].command;
                    goto allocate_actor;
                    case 0xFE:
                    temp_v0_6 = (ScriptSlot *)(object->slot * 0x23C);
                    temp_v0_6 = (ScriptSlot *)((s32)temp_v0_6 + (u8 *)slots);
                    temp_v0_6->command = (s32) temp_v0_6->current;
                    temp_a0_8 = object->slot;
                    var_a1_4 = slots[temp_a0_8].current;
                    goto allocate_actor;
allocate_actor:
                    func_8009615C(temp_a0_8, var_a1_4);
advance_actor_command:
                    clear_slot = object->slot;
                    cursor += 1;
clear_actor_state:
                    temp_v0_4 = (u8 *)&slots[clear_slot];
                    temp_v0_4->state = (s32) (temp_v0_4->state & ~0x1800);
                }
next_command:
                command_slot = object->slot;
        bank_offset = script_offset + players[command_slot].program * 0x300;
        opcode_ptr = bank_offset + programs + cursor;
                opcode = *opcode_ptr;
                if (opcode < 0xEBU)
                {
                    var_v0 = 0;
                    goto apply_frame;
                }
                goto dispatch_command;
            }
            else
            {
                goto sequence_finished;
            }
        }
        else
        {
apply_frame:
            {
                u8 *frame_programs;
                ScriptPlayer *frame_players;
                ScriptSlot *frame_slots;
                s32 frame_address;
                s32 frame_offset;
                frame_programs = D_8010AED0;
                frame_players = D_800FD818;
                frame_offset = (script_index << 5) + frame_players[object->slot].program * 0x300;
                frame_address = frame_offset;
                frame_address += (s32)frame_programs;
                frame_address += cursor;
                cursor++;
                frame_slots = D_80105AE0;
                object->state = *(u8 *)frame_address + (object->state & 0x80);
                frame_slots[object->slot].cursor = cursor;
            }
            object->delay = 1;
            object->progress = 0;
            object->active = 1;
            /* Duplicate return node #61. Try simplifying control flow for better match */
            return 0;
        }
    }
    else
    {
        return 1;
    }
}

#undef ACCESS_U8
#undef ACCESS_U16
#undef ACCESS_S32


/* func_8009615C */
#include "common.h"

/** @brief Animation record fields consumed by actor initialization. */
typedef struct
{
    u8 pad[0x12];
    u16 value;
    u8 pad14[2];
    u8 mode;
    u8 pad17[5];
} Animation;
/** @brief CommandView4_Actor slot fields initialized from a template. */
typedef struct
{
    u8 pad[0xC];
    Animation *current, *animations;
    u8 pad14[0x10];
    u8 active;
    u8 pad25[4];
    u8 animation;
    u8 pad_2a[0x1F8];
    u16 value;
    u8 pad224[15];
    u8 slot;
    u8 pad234[4];
    u16 mode;
    u8 pad_23a[10];
} CommandView4_Actor;
/** @brief Source actor template with its original stride. */
typedef struct
{
    u8 data[0x268];
} Template;
/** @brief Source record containing the assigned actor slot. */
typedef struct
{
    u8 pad[0x179];
    u8 slot;
    u8 tail[0xC2];
} Record;
/** @brief One of three actor-slot bindings. */
typedef struct
{
    u8 pad[0x18];
    s32 slot;
} Binding;




extern s32 func_800839F8(s32, s32);
extern void bcopy(void *, void *, s32);
/**
 * @brief Copy an actor template into a slot and select its initial animation.
 * @param index Source template and record index.
 * @param flags Animation override flag and two-bit animation index.
 * @return Assigned actor slot, or -1 when allocation fails.
 */
s32 func_8009615C(s32 index, s32 flags)
{
    s32 slot, binding;
    CommandView4_Actor *actor, *updated, *slots;
    Binding *bindings;
    slot = func_800839F8(index, 0);
    if (slot != -1)
    {
        actor = &((CommandView4_Actor *)g_field_actor_slots)[slot];
        bcopy(&((Template *)D_800FD81C)[index], actor, 0x244);
        actor->active = 1;
        actor->slot = slot;
        if (flags & 0x4000)
        {
            actor->animation = (flags >> 12) & 3;
            actor->value = actor->animations[actor->animation].value;
        }
        else
        {
            actor->animation = 0;
            actor->value = actor->animations->value;
        }
        slots = ((CommandView4_Actor *)g_field_actor_slots);
        updated = &slots[slot];
        binding = index;
        updated->mode = updated->animations[updated->animation].mode;
        updated->current = &updated->animations[updated->animation];
        ((Record *)D_80105AE0)[binding].slot = slot;
        bindings = ((Binding *)D_80105880);
        if (binding >= 3)
        {
            binding = 2;
        }
        bindings[binding].slot = slot;
    }
    else
    {
        ((Record *)D_80105AE0)[index].slot = 0xFF;
    }
    return slot;
}


/* field255 */
#include "common.h"

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;              // 0x24
    u8 pad25[0x27 - 0x25];
    u8 unk27;               // 0x27
    u8 pad28[0x2A - 0x28];
    s16 unk2A;              // 0x2A
    u8 pad2C[0x2E - 0x2C];
    s16 unk2E;               // 0x2E
    u8 pad30[0x3A - 0x30];
    u8 unk3A;                // 0x3A
    u8 pad3B[0x54 - 0x3B];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0x174];
    s32 unk174;              // 0x174
    u8 pad178[0x23C - 0x178];
} Struct_D80105AE0;



void func_8006C3FC(Struct_D800FDF58 *rec);

void func_80096334(Struct_D800FDF58 *a0)
{
    a0->unk2E = 1;
    a0->unk27 = 0;
    a0->unk24 = 1;

    ((Struct_D80105AE0 *)D_80105AE0)[a0->unk3A].unk174 &= ~0x1800;

    func_8006C3FC(a0);
}


/* func_80096394 */
#include "common.h"

/** @brief Presence byte in a 0x54-byte field actor record. */
typedef struct
{
    u8 pad[0x25];
    u8 presence;
    u8 tail[0x2E];
} FieldColorActor;
/** @brief Links, state, base color, and flash timer in a 0x23C-byte actor slot. */
typedef struct
{
    u32 unknown0;
    u32 link;
    u8 pad8[0x16C];
    u32 options;
    union
    {
        u32 word;
        struct
        {
            unsigned bit0 : 1;
            unsigned low : 4;
            unsigned bit5 : 1;
            unsigned bit6 : 1;
            unsigned bit7 : 1;
            unsigned high : 24;
        } bits;
    } state;
    u8 pad17c[0x2C];
    u8 red, green, blue, timer;
    u8 tail[0x90];
} FieldColorActorSlot;
/** @brief Rendered color in a 0x48-byte field visual record. */
typedef struct
{
    u8 pad[0xE];
    u8 red, green, blue;
    u8 tail[0x37];
} FieldColorVisual;
/** @brief Active state and actor index in a 0x1C-byte selection record. */
typedef struct
{
    s32 active;
    u8 pad4[8];
    s32 actor_index;
    u8 tail[0xC];
} FieldColorSelection;
extern FieldColorActor D_800FDF58[];



extern s32 D_800FE754;
extern s32 g_frame_counter;

/**
 * @brief Update actor tint colors and timed or selection-dependent flashing.
 * @note The thirteen slots retain separate byte and word reads of state flags.
 * @note A set timer/frame bit dims each base color to 100/128 of its value.
 */
void func_80096394(void)
{
    s32 i = 0;
    FieldColorVisual *visual = ((FieldColorVisual *)D_800FE3A0);
    FieldColorActor *actor = D_800FDF58;
    FieldColorActorSlot *slot = ((FieldColorActorSlot *)D_80105AE0);
    s32 selected;
    u32 flags;
    u32 options;
    u8 timer;

    do
    {
        slot = &((FieldColorActorSlot *)D_80105AE0)[i];
        visual = &((FieldColorVisual *)D_800FE3A0)[i];
        if (D_800FDF58[i].presence != 0xFF)
        {
            timer = slot->timer;
            if (timer != 0)
            {
                if (timer & 4)
                {
                    visual->red = (slot->red * 100) / 128;
                    visual->green = (slot->green * 100) / 128;
                    visual->blue = (slot->blue * 100) / 128;
                }
                else
                {
                    visual->red = slot->red;
                    visual->green = slot->green;
                    visual->blue = slot->blue;
                }
                timer = slot->timer - 1;
                slot->timer = timer;
                if (timer == 0)
                {
                    slot->options &= 0xFFFF7FFF;
                }
            }
            else
            {
                if (slot->link != 0 && !(*(u8 *)&slot->state.word & 1))
                {
                    flags = slot->state.word;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1))
                    {
                        if (!((flags >> 6) & 1))
                        {
                            selected = i;
                            if (i >= 3)
                            {
                                selected = 2;
                            }
                            if (((FieldColorSelection *)D_80105880)[selected].actor_index == i)
                            {
                                selected = i;
                                if (i >= 3)
                                {
                                    selected = 2;
                                }
                                if (((FieldColorSelection *)D_80105880)[selected].active != 0)
                                {
                                    goto check_blink;
                                }
                            }
                        }
                        slot->options &= 0xFFFF7FFF;
                        visual->red = slot->red;
                        goto restore_green;
                    }
                }
check_blink:
                options = slot->options & 0xFFFF7FFF;
                slot->options = options;
                if (D_800FDF58[i].presence != 0xFF && slot->link != 0 && !(*(u8 *)&slot->state.word & 1))
                {
                    flags = slot->state.word;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1) && D_800FE754 != 0)
                    {
                        slot->options = options | 0x8000;
                        if (g_frame_counter & 4)
                        {
                            ((FieldColorVisual *)D_800FE3A0)[i].red = (slot->red * 100) / 128;
                            ((FieldColorVisual *)D_800FE3A0)[i].green = (slot->green * 100) / 128;
                            ((FieldColorVisual *)D_800FE3A0)[i].blue = (slot->blue * 100) / 128;
                        }
                        else
                        {
                            visual->red = slot->red;
restore_green:
                            visual->green = slot->green;
                            visual->blue = slot->blue;
                        }
                    }
                }
            }
        }
        actor++;
        i++;
    } while (i < 13);
}
