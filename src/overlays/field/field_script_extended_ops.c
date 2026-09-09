#include "common.h"
#include "field_script.h"

/*
 * Extended field script opcodes 0x80 through 0x8F.
 *
 * field_script_run hands opcodes of 0x80 and above to func_800B8308, which
 * decodes up to four operands from the descriptor bytes that follow the opcode
 * and jumps through g_field_script_ext_op_table[opcode - 0x80]. Every handler
 * here receives those decoded operands in order. An operand of 0xFF in an
 * actor-id slot means the script owner.
 */

/** @brief View of D_80122B78 exposing the three packed 10-bit fields at 0x410. */
typedef struct
{
    u8 pad0[0x404];
    s32 unk404; /* 0x404 */
    u8 pad408[0x410 - 0x408];
    u32 unk410; /* 0x410 three 10-bit fields */
    u32 unk414; /* 0x414 */
} StructB78;

u8 *func_800C1E40(s32 arg0);
void func_800B2844(s32 arg0, void *arg1, s32 arg2);
void func_800B28E0(s32, s32, s32);
void func_800B286C(s32, s32, s32);
void func_800681E4(s32 arg0, s32 arg1, s32 arg2);
void akao_cmd_a9(s32 arg0, s32 arg1);
void func_80089D44(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void field_script_op_00(void);

extern u8 *D_80122B78;
extern s32 g_layout_option;
extern s32 g_layout_sub_mode;

void func_8006B8DC(s32, s32, s32, s32, s32);
void func_800C28B8(s32);
void func_80087A9C(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
void func_800B0710(s32, s32, s32, s32);

/**
 * @brief Forward four values to akao_set_song_params.
 * @param flags AKAO flags word.
 * @param duration Duration.
 * @param field_id Field id.
 * @param sub_id Sub id.
 */
void func_800BCB68(s32 flags, s32 duration, s32 field_id, s32 sub_id)
{
    akao_set_song_params(flags, duration, field_id, sub_id);
}

/**
 * @brief Split bit 7 of arg0 into a flag and forward the rest to func_8006B8DC.
 * @param arg0 Id with an optional 0x80 flag bit.
 * @param arg1 Forwarded as the first argument.
 * @param arg2 Forwarded as the second argument.
 * @param arg3 Forwarded as the third argument.
 */
void func_800BCB88(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 flag;

    if (arg0 & 0x80)
    {
        flag = 1;
        arg0 &= 0x7F;
    }
    else
    {
        flag = 0;
    }
    func_8006B8DC(arg1, arg2, arg3, flag, arg0);
}

/**
 * @brief Split bit 7 of arg0 into a flag, run func_800C28B8 on the id, then forward everything to func_80087A9C.
 * @param arg0 Actor id with an optional 0x80 flag bit.
 * @param arg1 Forwarded to func_80087A9C.
 * @param arg2 Forwarded to func_80087A9C.
 * @param arg3 Forwarded to func_80087A9C.
 */
void func_800BCBD0(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 var_s0;
    s32 var_s1;

    if (arg0 & 0x80)
    {
        var_s1 = 1;
        var_s0 = arg0 & 0x7F;
    }
    else
    {
        var_s1 = 0;
        var_s0 = arg0;
    }
    func_800C28B8(var_s0);
    func_80087A9C(var_s0, arg1, arg2, arg3, 0, -1, -1, -1, 0, var_s1);
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCC6C(void)
{
}

/**
 * @brief Forward four values to func_800B0710, mapping 0xFF to the script owner in the first and to -1 in the rest.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 0xFF becomes -1.
 * @param arg2 0xFF becomes -1.
 * @param arg3 0xFF becomes -1.
 */
void func_800BCC74(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 v;

    if (arg0 == 0xFF)
    {
        v = g_field_script->status.owner_id;
    }
    else
    {
        v = arg0;
    }
    func_800B0710(v,
                  (arg1 == 0xFF) ? -1 : arg1,
                  (arg2 == 0xFF) ? -1 : arg2,
                  (arg3 == 0xFF) ? -1 : arg3);
}


/** @brief Pending layout transition state with overlapping control fields at 0x418. */
typedef struct
{
    u8 pad[0x404];
    s32 layout, option, third_selector;
    u8 pad410[8];
    union
    {
        s32 word;
        struct
        {
            s16 id;
            s8 mode;
            u8 flags;
        } fields;
    } control;
} State;
extern u8 *D_80122B78;
extern s32 g_layout_flag;
extern s32 g_layout_option;

/**
 * @brief Set pending layout selectors and reset execution to script record zero.
 * @param transition_id Transition identifier stored in the control word's low halfword.
 * @param mode Mode byte stored at offset 0x41A.
 * @param selectors Three packed selector bytes; 0xFE and 0xFF select sentinel behavior.
 * @param flags Five-bit transition flags stored at control bits 24 through 28.
 * @note 100% match with GCC 2.7.2 CDK and GCC 2.8.0 G0: 109 instructions, 436 bytes.
 */
void func_800BCCE0(s16 transition_id, s8 mode, s32 selectors, s32 flags)
{
    s32 layout;
    s32 third_selector;
    s32 option;

    State *state = (State *)D_80122B78;
    option = selectors & 0xFF;
    state->control.word = (s32)(state->control.word | 0x40000000);
    state->control.fields.mode = mode;
    layout = (selectors >> 8) & 0xFF;
    third_selector = (selectors >> 0x10) & 0xFF;
    state->control.fields.id = transition_id;
    switch (option)
    {
    case 0xFE:
        ((State *)D_80122B78)->option = -2;
        break;
    case 0xFF:
        g_layout_option = -1;
        ((State *)D_80122B78)->option = -1;
        break;
    default:
        if (option == g_layout_option)
        {
            ((State *)D_80122B78)->option = -2;
        }
        else
        {
            ((State *)D_80122B78)->option = option;
        }
        break;
    }
    switch (layout)
    {
    case 0xFE:
        ((State *)D_80122B78)->layout = -2;
        break;
    case 0xFF:
        ((State *)D_80122B78)->layout = -1;
        break;
    default:
        if (layout == g_layout_flag)
        {
            ((State *)D_80122B78)->layout = -1;
        }
        else
        {
            ((State *)D_80122B78)->layout = layout;
        }
        break;
    }
    switch (third_selector)
    {
    case 0xFE:
        ((State *)D_80122B78)->third_selector = -2;
        break;
    case 0xFF:
        ((State *)D_80122B78)->third_selector = -1;
        break;
    default:
        ((State *)D_80122B78)->third_selector = third_selector;
        break;
    }
    ((State *)D_80122B78)->control.word =
        (s32)((((State *)D_80122B78)->control.word & 0xE0FFFFFF) | ((flags & 0x1F) << 0x18));
    g_field_script->active_record = 0;
    g_field_script->status.word = (s32)(g_field_script->status.word & 0x7FFFFFFF);
    ((FieldScriptRecord *)((u8 *)g_field_script + ((g_field_script->active_record * 3) << 2)))->pc =
        0;
}

/**
 * @brief Opcode 0x86: dispatch an entry of a resource record through func_800B2844.
 *
 * Fetches the record for @p resource_id via func_800C1E40; when non-NULL, reads
 * the halfword at @c entry_index*2 + 4 within it and calls func_800B2844 with
 * the record address offset by that halfword plus 4.
 *
 * @param operand_0 Forwarded to func_800B2844 as its first argument.
 * @param resource_id Record selector passed to func_800C1E40.
 * @param entry_index Halfword index within the record (scaled by 2).
 * @param operand_3 Forwarded to func_800B2844 as its third argument.
 */
void field_script_op_86(s32 operand_0, s32 resource_id, s32 entry_index, s32 operand_3)
{
    u8 *p = func_800C1E40(resource_id);

    if (p != NULL)
    {
        u16 h = *(u16 *)(p + (entry_index << 1) + 4);
        func_800B2844(operand_0, p + (h + 4), operand_3);
    }
}

/**
 * @brief Opcode 0x87: route an actor to func_800B28E0 or func_800B286C by selector.
 *
 * Selector 0 forwards to func_800B28E0 and selector 1 to func_800B286C, each
 * with the resolved actor id and the low bytes of the last two operands.
 *
 * @param selector Handler selector, 0 or 1.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param operand_2 Low byte forwarded to the handler.
 * @param operand_3 Low byte forwarded to the handler.
 */
void field_script_op_87(s32 selector, s32 actor_id, s32 operand_2, s32 operand_3)
{
    if (actor_id == 0xFF)
    {
        actor_id = g_field_script->status.owner_id;
    }
    switch (selector)
    {
    case 0:
        func_800B28E0(actor_id, operand_2 & 0xFF, operand_3 & 0xFF);
        break;
    case 1:
        func_800B286C(actor_id, operand_2 & 0xFF, operand_3 & 0xFF);
        break;
    }
}

/**
 * @brief Opcode 0x88: reset the layout option globals and start the func_800681E4 transition.
 * @param operand_0 Forwarded unchanged.
 * @param operand_1 0xFF becomes -1.
 * @param operand_2 0xFF becomes -1.
 */
void field_script_op_88(s32 operand_0, s32 operand_1, s32 operand_2)
{
    s32 var_v0;
    s32 var_a3;

    var_v0 = -1;
    if (operand_1 != 0xFF)
    {
        var_v0 = operand_1;
    }
    operand_1 = var_v0;

    var_a3 = -1;
    if (operand_2 != 0xFF)
    {
        var_a3 = operand_2;
    }

    g_layout_option = -1;

    operand_2 = var_a3;

    g_layout_sub_mode = -1;

    func_800681E4(operand_0, operand_1, operand_2);
}

/**
 * @brief Opcode 0x89: issue AKAO command 0xA9 with a minimum first value of 1.
 * @param operand_0 Unused.
 * @param operand_1 Unused.
 * @param value First AKAO operand; 0 is promoted to 1.
 * @param operand_3 Second AKAO operand.
 */
void field_script_op_89(s32 operand_0, s32 operand_1, s32 value, s32 operand_3)
{
    if (value == 0)
    {
        value = 1;
    }
    akao_cmd_a9(value, operand_3);
}

/**
 * @brief Opcode 0x8A: forward an actor pair to func_8008B5D0.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param operand_1 Forwarded unchanged.
 * @param target_id Target id, or 0xFF for the script owner; passed by address.
 * @param operand_3 Unused.
 */
void field_script_op_8a(s32 actor_id, s32 operand_1, s32 target_id, s32 operand_3)
{
    s32 resolved_target;
    s32 resolved_actor;

    resolved_actor = actor_id;
    if (target_id == 0xFF)
    {
        resolved_target = (s32) g_field_script->status.owner_id;
    }
    else
    {
        resolved_target = target_id;
    }
    if (resolved_actor == 0xFF)
    {
        resolved_actor = g_field_script->status.owner_id;
    }
    func_8008B5D0(resolved_actor, operand_1, 1, &resolved_target);
}

/**
 * @brief Opcode 0x8B: pack three 10-bit fields into the word at 0x410 and store the fourth operand at 0x414.
 * @param field_0 Bits 0-9.
 * @param field_1 Bits 10-19.
 * @param field_2 Bits 20-29.
 * @param operand_3 Stored to unk414.
 */
void field_script_op_8b(s32 field_0, s32 field_1, s32 field_2, s32 operand_3)
{
    StructB78 *p;
    u32 raw;
    u32 v;

    p = (StructB78 *)D_80122B78;
    raw = p->unk410;
    p->unk414 = operand_3;
    v = raw;
    v &= ~0x3FF;
    v |= field_0 & 0x3FF;
    v &= 0xFFF003FF;
    v |= (field_1 & 0x3FF) << 10;
    v &= 0xC00FFFFF;
    v |= (field_2 & 0x3FF) << 20;
    p->unk410 = v;
}

/**
 * @brief Opcode 0x8C: forward to func_80089D44 with 0xFF operands mapped to the owner id or -1.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param operand_1 0xFF becomes -1.
 * @param operand_2 0xFF becomes -1.
 * @param operand_3 0xFF becomes -1.
 */
void field_script_op_8c(s32 actor_id, s32 operand_1, s32 operand_2, s32 operand_3)
{
    s32 resolved_actor;

    if (actor_id == 0xFF)
    {
        resolved_actor = g_field_script->status.owner_id;
    }
    else
    {
        resolved_actor = actor_id;
    }
    func_80089D44(resolved_actor,
                  (operand_1 == 0xFF) ? -1 : operand_1,
                  (operand_2 == 0xFF) ? -1 : operand_2,
                  (operand_3 == 0xFF) ? -1 : operand_3);
}

/**
 * @brief Opcode 0x8D: no-op.
 */
void field_script_op_8d(void)
{
}

/**
 * @brief Opcode 0x8E: no-op.
 */
void field_script_op_8e(void)
{
}

/**
 * @brief Opcode 0x8F: no-op.
 */
void field_script_op_8f(void)
{
}
