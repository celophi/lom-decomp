#include "common.h"
#include "field_script.h"

/** @brief View of D_80122B78 exposing the packed position word at 0x410. */
typedef struct
{
    u8 pad0[0x404];
    s32 unk404; /* 0x404 */
    u8 pad408[0x410 - 0x408];
    u32 unk410; /* 0x410 */
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

extern StructB78 *D_80122B78;
extern s32 g_layout_option;
extern s32 g_layout_sub_mode;

/**
 * @brief Looks up a record and forwards a derived address to func_800B2844.
 *
 * Fetches the record for @p arg1 via func_800C1E40; when non-NULL, reads the
 * halfword at @c arg2*2 + 4 within it and calls func_800B2844 with @p arg0, the
 * record address offset by that halfword plus 4, and @p arg3.
 *
 * @param arg0 Forwarded to func_800B2844.
 * @param arg1 Record selector passed to func_800C1E40.
 * @param arg2 Halfword index within the record (scaled by 2).
 * @param arg3 Forwarded to func_800B2844.
 */
void func_800BCE94(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    u8 *p = func_800C1E40(arg1);

    if (p != NULL)
    {
        u16 h = *(u16 *)(p + (arg2 << 1) + 4);
        func_800B2844(arg0, p + (h + 4), arg3);
    }
}

/**
 * @brief Dispatches to one of two field handlers based on a selector.
 *
 * If @p p1 is the 0xFF sentinel it is replaced by the script owner's id.
 * Selector 0 forwards to func_800B28E0 and selector 1 to func_800B286C,
 * each with the resolved value and the low bytes of @p p2 and @p p3.
 *
 * @param p0 Handler selector, 0 or 1.
 * @param p1 Actor id, or 0xFF for the script owner.
 * @param p2 Low byte forwarded to the handler.
 * @param p3 Low byte forwarded to the handler.
 */
void func_800BCEFC(s32 p0, s32 p1, s32 p2, s32 p3)
{
    if (p1 == 0xFF)
    {
        p1 = g_field_script->status.owner_id;
    }
    switch (p0)
    {
    case 0:
        func_800B28E0(p1, p2 & 0xFF, p3 & 0xFF);
        break;
    case 1:
        func_800B286C(p1, p2 & 0xFF, p3 & 0xFF);
        break;
    }
}

/**
 * @brief Reset the layout option globals and forward to func_800681E4 with 0xFF mapped to -1.
 * @param arg0 Forwarded unchanged.
 * @param arg1 0xFF becomes -1.
 * @param arg2 0xFF becomes -1.
 */
void func_800BCF68(s32 arg0, s32 arg1, s32 arg2)
{
    s32 var_v0;
    s32 var_a3;

    var_v0 = -1;
    if (arg1 != 0xFF)
    {
        var_v0 = arg1;
    }
    arg1 = var_v0;

    var_a3 = -1;
    if (arg2 != 0xFF)
    {
        var_a3 = arg2;
    }

    g_layout_option = -1;

    arg2 = var_a3;

    g_layout_sub_mode = -1;

    func_800681E4(arg0, arg1, arg2);
}

/**
 * @brief Issue AKAO command 0xA9 with a minimum first operand of 1.
 * @param arg0 Unused.
 * @param arg1 Unused.
 * @param arg2 First operand; 0 is promoted to 1.
 * @param arg3 Second operand.
 */
void func_800BCFBC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    if (arg2 == 0)
    {
        arg2 = 1;
    }
    akao_cmd_a9(arg2, arg3);
}

/**
 * @brief Forward an actor pair to func_8008B5D0, mapping 0xFF to the script owner.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Forwarded unchanged.
 * @param arg2 Target id, or 0xFF for the script owner; passed by address.
 * @param arg3 Unused.
 */
void func_800BCFEC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 sp10;
    s32 var_a0;

    var_a0 = arg0;
    if (arg2 == 0xFF)
    {
        sp10 = (s32) g_field_script->status.owner_id;
    }
    else
    {
        sp10 = arg2;
    }
    if (var_a0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    func_8008B5D0(var_a0, arg1, 1, &sp10);
}

/**
 * @brief Pack three 10-bit fields into the word at 0x410 and store arg3 at 0x414.
 * @param arg0 Bits 0-9.
 * @param arg1 Bits 10-19.
 * @param arg2 Bits 20-29.
 * @param arg3 Stored to unk414.
 */
void func_800BD04C(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    StructB78 *p;
    u32 raw;
    u32 v;

    p = D_80122B78;
    raw = p->unk410;
    p->unk414 = arg3;
    v = raw;
    v &= ~0x3FF;
    v |= arg0 & 0x3FF;
    v &= 0xFFF003FF;
    v |= (arg1 & 0x3FF) << 10;
    v &= 0xC00FFFFF;
    v |= (arg2 & 0x3FF) << 20;
    p->unk410 = v;
}

/**
 * @brief Forward to func_80089D44 with 0xFF operands mapped to the owner id or -1.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 0xFF becomes -1.
 * @param arg2 0xFF becomes -1.
 * @param arg3 0xFF becomes -1.
 */
void func_800BD0A4(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
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
    func_80089D44(v,
                  (arg1 == 0xFF) ? -1 : arg1,
                  (arg2 == 0xFF) ? -1 : arg2,
                  (arg3 == 0xFF) ? -1 : arg3);
}

/**
 * @brief Empty function; no-op.
 */
void func_800BD110(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BD118(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BD120(void)
{
}

/**
 * @brief Apply a signed 16-bit relative jump to the active record's program counter.
 *
 * Reads a little-endian 16-bit delta from the active record's program counter
 * at offset @p arg0. A non-zero delta advances the pc by it (sign-extended via
 * the 0x8000 bit); a zero delta hands off to field_script_op_00 to step the
 * cursor.
 *
 * @param arg0 Byte offset from the program counter holding the delta.
 * @see decomp.me (100%) TODO
 */
void field_script_branch(s32 arg0)
{
    FieldScriptRecord *rec;
    s32 unk8;
    u8 *ptr;
    s32 val;
    s32 lo;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    unk8 = (s32)rec->pc;
    ptr = (u8 *)(unk8 + arg0);
    val = ptr[0] + (ptr[1] << 8);
    lo = val & 0xFFFF;
    if (lo != 0)
    {
        if (val & 0x8000)
        {
            s32 t = unk8 + 0xFFFF0000;
            rec->pc = (u8 *)(t + lo);
            return;
        }
        rec->pc = (u8 *)(unk8 + lo);
        return;
    }
    field_script_op_00();
}

/**
 * @brief Read one operand, mapping the value 0xFF to the script owner's id.
 * @param type Operand type from the descriptor byte.
 * @param data Operand stream position.
 * @param value Receives the decoded value.
 * @return The advanced operand stream position.
 */
u8 *field_script_read_operand_or_owner(u32 type, u8 *data, s32 *value)
{
    u8 *result;

    result = field_script_read_operand(type, data, value);
    if (*value == 0xFF)
    {
        *value = g_field_script->status.owner_id;
    }
    return result;
}
