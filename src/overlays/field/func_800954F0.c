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
extern ScriptBinding D_80105880[];
extern ScriptSlot D_80105AE0[];
extern u8 D_8010AED0[];
extern ScriptActor g_field_actor_slots[];

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
    s32 parameters[14];
    void *copy_source;
    u8 *var_a1;
    s32 *var_a1_2;
    s32 *var_a1_3;
    s32 temp_a0_9;
    s32 temp_s0;
    s32 script_offset;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a1_4;
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
    u8 temp_a0_8;
    u8 command;
    u8 temp_a2;
    u8 temp_a2_2;
    u8 temp_v1_3;
    u8 opcode;
    ScriptSlot *temp_a0;
    ScriptSlot *temp_a0_10;
    ScriptSlot *temp_a0_6;
    ScriptSlot *temp_a1;
    ScriptSlot *temp_v0;
    ScriptSlot *temp_v0_4;
    ScriptSlot *temp_v0_5;
    ScriptSlot *temp_v0_6;
    ScriptActor *temp_v1_2;
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
                temp_v1_2 = (temp_a1->actor * 0x244) + (u8 *)g_field_actor_slots;
                if ((temp_v1_2->active != 0) && (temp_v1_2->owner == temp_a2))
                {
                    temp_v1_2->active = 0U;
                }
            }
            D_80105AE0[object->slot].command = 0xFFFF;
        }
    }
    temp_a2_2 = object->slot;
    cursor = D_80105AE0[temp_a2_2].cursor;
    initial_program = (script_index << 5) + (D_800FD818[temp_a2_2].program * 0x300) + D_8010AED0 + cursor;
    var_v0 = 1;
    if (*initial_program != 0xFF)
    {
        if (cursor == 1)
        {
            if (temp_a2_2 < 2U)
            {
                var_v0_2 = temp_a2_2 * 0x1C;
            }
            else
            {
                var_v0_2 = 0x38;
            }
            var_v0 = 0;
            if (((ScriptBinding *)((u8 *)D_80105880 + var_v0_2))->state == 2)
            {
                ((ScriptActor *)(((((ScriptBinding *)(((u8 *)&D_80105880[object->slot])))->actor * 0x244) + (u8 *)g_field_actor_slots)))->mode = 0;
                ((ScriptActor *)(((((ScriptBinding *)(((u8 *)&D_80105880[object->slot])))->actor * 0x244) + (u8 *)g_field_actor_slots)))->count = 0;
                if ((u8) object->slot < 2U)
                {
                    var_v0_3 = object->slot * 0x1C;
                }
                else
                {
                    var_v0_3 = 0x38;
                }
                copy_source = (((ScriptBinding *)(((u8 *)D_80105880 + var_v0_3)))->actor * 0x244) + (u8 *)g_field_actor_slots;
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
                if ((u8) object->slot < 2U)
                {
                    var_v0_4 = object->slot * 0x1C;
                    goto release_restored_actor;
                }
                goto fallback_binding;

            }
            /* Duplicate return node #61. Try simplifying control flow for better match */
            return var_v0;
        }
        goto begin_commands;
sequence_finished:

        D_80105AE0[object->slot].cursor = cursor;
        object->delay = 1;
        object->progress = 0;
        object->active = 1;
        object->state = (u8) (object->state & 0x80);
        return 0;

fallback_binding:
        var_v0_4 = 0x38;
release_restored_actor:
        ((ScriptActor *)(((u8 *)g_field_actor_slots + (((ScriptBinding *)(((u8 *)D_80105880 + var_v0_4)))->actor * 0x244))))->active = 0;
begin_commands:
        programs = D_8010AED0;
        players = D_800FD818;
        slots = D_80105AE0;
        script_offset = script_index << 5;
        opcode_ptr = script_offset + (players[object->slot].program * 0x300) + (programs + cursor);
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
                    temp_a0 = (u8 *)&slots[object->slot];
                    temp_a0->command = (s32) ((temp_a0->current & 0x3FF) | (((command - 0xEB) << 0xC) | 0x8000) | 0x4000);
                    temp_a0_2 = object->slot;
                    temp_s0 = func_8009615C(temp_a0_2, slots[temp_a0_2].command);
                    var_a0_2 = 0;
                    if (slots[object->slot].count != 0)
                    {
                        var_a1_2 = parameters;
                        do
                        {
                            *var_a1_2 = (s32) ((ScriptSlot *)(((u8 *)&slots[object->slot] + var_a0_2)))->arguments[0];
                            var_a0_2 += 1;
                            var_a1_2++;
                        } while (var_a0_2 < (s32) slots[object->slot].count);
                    }
                    field_start_actor_animation(temp_s0, slots[object->slot].count, (u8 *)parameters);
                    cursor += 1;
                    slots[object->slot].command = 0xFFFF;
                    case 0xEE:
                    temp_v0 = (u8 *)&slots[object->slot];
                    temp_v0->command = (s32) temp_v0->current;
                    temp_a0_3 = object->slot;
                    temp_v0_2 = func_8009615C(temp_a0_3, slots[temp_a0_3].current);
                    slots[object->slot].command = 0xFFFF;
                    var_a0_3 = 0;
                    if (slots[object->slot].count != 0)
                    {
                        var_a1_3 = parameters;
                        do
                        {
                            *var_a1_3 = (s32) ((ScriptSlot *)(((u8 *)&slots[object->slot] + var_a0_3)))->arguments[0];
                            var_a0_3 += 1;
                            var_a1_3++;
                        } while (var_a0_3 < (s32) slots[object->slot].count);
                    }
                    cursor += 1;
                    field_start_actor_animation(temp_v0_2, slots[object->slot].count, (u8 *)parameters);
                    goto next_command;
                    case 0xF0:
                    temp_a0_4 = object->slot;
                    slots[temp_a0_4].delay = (u8) ((ScriptPlayer *)((script_offset + (((ScriptPlayer *)(((u8 *)&players[temp_a0_4])))->program * 0x300) + (programs + cursor))))->program;
                    cursor += 2;
                    goto save_cursor;
                    case 0xEF:
                    case 0xF1:
                    goto save_cursor;
save_cursor:
                    slots[object->slot].cursor = cursor;
                    return 0;
                    case 0xF2:
                    var_v1_2 = (u8 *)&slots[object->slot];
                    var_v0_5 = var_v1_2->flags ^ 0x4000;
                    goto refresh_actor_flags;
                    case 0xF3:
                    var_v1_2 = (u8 *)&slots[object->slot];
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
                    temp_v0_3 = func_800839F8(object->slot, 0);
                    if (temp_v0_3 != -1)
                    {
                        temp_a0_5 = object->slot;
                        func_80083EEC(temp_a0_5, temp_v0_3, ((ScriptPlayer *)((script_offset + (((ScriptPlayer *)(((u8 *)&players[temp_a0_5])))->program * 0x300) + (programs + cursor))))->program);
                        field_start_actor_animation(temp_v0_3, 0U, NULL);
                    }
                    cursor += 2;
                    goto clear_actor_state;
                    case 0xF6:
                    case 0xF7:
                    case 0xF8:
                    temp_a0_6 = (u8 *)&slots[object->slot];
                    temp_a0_6->command = (s32) ((temp_a0_6->current & 0x3FF) | (((command - 0xF6) << 0xC) | 0x8000) | 0x4000);
                    temp_a0_7 = object->slot;
                    field_start_actor_animation(func_8009615C(temp_a0_7, slots[temp_a0_7].command), 0U, NULL);
                    slots[object->slot].command = 0xFFFF;
                    goto advance_actor_command;
                    case 0xF9:
                    temp_v0_5 = (u8 *)&slots[object->slot];
                    temp_v0_5->command = (s32) temp_v0_5->current;
                    temp_a0_8 = object->slot;
                    temp_a0_9 = func_8009615C(temp_a0_8, slots[temp_a0_8].current);
                    slots[object->slot].command = 0xFFFF;
                    field_start_actor_animation(temp_a0_9, 0U, NULL);
                    goto advance_actor_command;
                    case 0xFA:
                    temp_v1_3 = object->slot;
                    slots[temp_v1_3].command = (s32) ((ScriptPlayer *)((script_offset + (((ScriptPlayer *)(((u8 *)&players[temp_v1_3])))->program * 0x300) + (programs + cursor))))->program;
                    cursor += 2;
                    goto clear_actor_state;
                    case 0xFB:
                    case 0xFC:
                    case 0xFD:
                    temp_a0_10 = (u8 *)&slots[object->slot];
                    temp_a0_10->command = (s32) ((temp_a0_10->current & 0x3FF) | (((command - 0xFB) << 0xC) | 0x8000) | 0x4000);
                    var_a1_4 = slots[object->slot].command;
                    goto allocate_actor;
                    case 0xFE:
                    temp_v0_6 = (u8 *)&slots[object->slot];
                    temp_v0_6->command = (s32) temp_v0_6->current;
                    var_a1_4 = slots[object->slot].current;
                    goto allocate_actor;
allocate_actor:
                    func_8009615C(object->slot, var_a1_4);
advance_actor_command:
                    cursor += 1;
clear_actor_state:
                    temp_v0_4 = (u8 *)&slots[object->slot];
                    temp_v0_4->state = (s32) (temp_v0_4->state & ~0x1800);
                }
next_command:
                opcode_ptr = script_offset + (players[object->slot].program * 0x300) + (programs + cursor);
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
            object->state = (u8) (*((script_index << 5) + (players[object->slot].program * 0x300) + (programs + cursor)) + (object->state & 0x80));
            slots[object->slot].cursor = (s32) (cursor + 1);
            object->delay = 1;
            object->progress = 0;
            object->active = 1;
            /* Duplicate return node #61. Try simplifying control flow for better match */
            return var_v0;
        }
    }
    else
    {
        return var_v0;
    }
}

#undef ACCESS_U8
#undef ACCESS_U16
#undef ACCESS_S32
