#include "common.h"
#include "game_audio.h"

/*
 * Field script opcode handlers 0x1E through 0x36.
 *
 * field_script_run executes byte-coded scripts. Each step reads the opcode at
 * the active record's program counter; opcodes below 0x40 dispatch through
 * g_field_script_op_table, whose entries are the functions below. A handler
 * must advance the program counter past its operands. Clearing
 * FIELD_SCRIPT_RUNNING in the context status word ends the step loop for
 * this frame.
 *
 * Operands follow a descriptor byte at pc[1] that packs one two-bit type per
 * operand, high bits first: 0 reads a script variable, 1 a byte, 2 a
 * halfword, 3 a word. field_script_read_operand decodes one operand and
 * returns the advanced pointer; field_script_read_operand_or_owner also maps
 * the value 0xFF to the script owner's id.
 */

#define FIELD_SCRIPT_RUNNING 0x80000000

/* Operand types packed into the descriptor byte. */
#define OPERAND_TYPE_0(descriptor) ((descriptor) >> 6)
#define OPERAND_TYPE_1(descriptor) (((descriptor) >> 4) & 3)
#define OPERAND_TYPE_2(descriptor) (((descriptor) >> 2) & 3)
#define OPERAND_TYPE_3(descriptor) ((descriptor) & 3)

/** @brief One script record: the program counter lives at offset 8. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u8* pc;
} FieldScriptRecord;

/**
 * @brief Script context header. Records follow at a 12-byte stride from the
 *        base, so record 0 aliases this header and its pc sits at offset 8.
 */
typedef struct
{
    union
    {
        u32 word;
        u8 owner_id;
    } status;
    s32 active_record;
    u8* pc;
} FieldScriptContext;

extern FieldScriptContext* g_field_script;
extern u8* D_80122B74;
extern u8* D_80122B78;

#define FIELD_SCRIPT_RECORD(index) ((FieldScriptRecord*)((u8*)g_field_script + ((index) * 3 << 2)))
#define FIELD_SCRIPT_ACTIVE_RECORD() FIELD_SCRIPT_RECORD(g_field_script->active_record)

u8* field_script_read_operand(u32 type, u8* data, s32* value);
u8* field_script_read_operand_or_owner(u32 type, u8* data, s32* value);
void func_800B4410(s32 arg0);
void func_800B4584(void);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
void func_8006AB38(s32 arg0);
void func_800A43E8(s32 arg0, s32 arg1, u16 arg2, s32 arg3);
void func_800B286C(s32 arg0, s32 arg1, s32 arg2);
s32 func_800A4744(void);
u8 func_800A4778(void);
void func_800675C8(s32 arg0, s32 arg1, s32 arg2);
void func_800674D8(s32 arg0);
void func_8008AFD8(s32 arg0, s32 arg1, FieldScriptRecord* record, s32 record_index);
void func_80087614(s32 arg0, s32 arg1);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_800B2654(s32* arg0, s32* arg1, s32* arg2, s32* arg3);
void func_8009C620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8009C77C(s32 arg0, s32 arg1, s32 arg2);
void func_800A3988(s32 arg0, s32 arg1, s32 arg2, FieldScriptRecord* record);
void func_8008B5D0(s32 arg0, s32 arg1, s32 arg2, s32* arg3);

/**
 * @brief Opcode 0x1E: forward the byte operand to func_800B4410.
 */
void field_script_op_1e(void)
{
    s32 i;
    FieldScriptRecord* rec;

    rec = (FieldScriptRecord*)g_field_script;
    i = g_field_script->active_record;
    rec += i;
    func_800B4410(rec->pc[1]);

    rec = (FieldScriptRecord*)g_field_script;
    i = g_field_script->active_record;
    rec += i;
    rec->pc += 2;
}

/**
 * @brief Opcode 0x1F: call func_800B4584 and step past the opcode.
 */
void field_script_op_1f(void)
{
    FieldScriptRecord* rec;

    func_800B4584();
    rec = (FieldScriptRecord*)g_field_script + g_field_script->active_record;
    rec->pc = rec->pc + 1;
}

/**
 * @brief Opcode 0x20: queue audio sub-command 0x20 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_20(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x20);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x21: queue audio sub-command 0x21 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_21(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x21);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x22: queue audio sub-command 0x23 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter. Shares sub-command 0x23 with opcode 0x23.
 */
s32 field_script_op_22(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x23);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x23: queue audio sub-command 0x23 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_23(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x23);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x24: queue audio sub-command 0x24 for the owner and step past the opcode.
 */
void field_script_op_24(void)
{
    FieldScriptContext* ctx = g_field_script;
    FieldScriptRecord* rec;

    akao_set_song_params(0x8001, 1, ctx->status.owner_id, 0x24);

    rec = (FieldScriptRecord*)g_field_script;
    rec += g_field_script->active_record;
    rec->pc += 1;
}

/**
 * @brief Opcode 0x25: queue audio sub-command 0x25 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_25(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x25);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x26: queue audio sub-command 0x26 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_26(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x26);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x27: reset one of two D_80122B74 slots chosen by the byte operand, then call func_8006AB38.
 * @note Operand 0 selects the slot at 0x840 and also issues the 0xF87-based command; any other value selects 0xA90.
 */
void field_script_op_27(void)
{
    s32 state;

    state = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    if (state == 0)
    {
        D_80122B74[0x840] = 0;
        *(s32*)(D_80122B74 + 0x858) |= 0x7F;
        func_800BD520(0, (D_80122B74[0x859] << 3) + 0xF87, 0);
        func_800BD520(0, 0x2F08, 0xFF);
    }
    else
    {
        D_80122B74[0xA90] = 0;
        *(s32*)(D_80122B74 + 0xAA8) |= 0x7F;
        func_800BD520(0, 0x2F00, 0xFF);
    }
    func_8006AB38(state);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
}

/**
 * @brief Opcode 0x28: read three operands and pass them to func_800A43E8, then claim command slot 0xF.
 * @note The low two descriptor bits are passed as the first argument; a third operand of 0xFF becomes -1.
 */
void field_script_op_28(void)
{
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    if (arg2 == 0xFF)
    {
        arg2 = -1;
    }
    func_800A43E8(descriptor & 3, arg0, (u16)arg1, arg2);
    func_800B286C(0x80, 0, 0xF);
}

/**
 * @brief Opcode 0x29: issue command 0x7100 with func_800A4744's result, or end the step loop when it is negative.
 * @note On success claims command slot 0x10 and steps past the opcode; on failure claims the slot func_800A4778 names.
 */
void field_script_op_29(void)
{
    s32 result;
    FieldScriptRecord* rec;

    result = func_800A4744();
    if (result < 0)
    {
        func_800B286C(0x80, 0, func_800A4778() & 0xFF);
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    }
    func_800BD520(0, 0x7100, result);
    func_800B286C(0x80, 0, 0x10);
    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    rec->pc += 1;
}

/**
 * @brief Opcode 0x2A: consume four operands without acting on them.
 * @note A fourth operand of 0xFF is normalised to -1 but the values are otherwise unused.
 */
void field_script_op_2a(void)
{
    s32 arg3;
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg3);
    if (arg3 == 0xFF)
    {
        arg3 = -1;
    }
}

/**
 * @brief Opcode 0x2B: render a number into a text entry.
 * @note Operands are entry index, value and digit count; a zero digit count is replaced by the value's decimal length.
 */
void field_script_op_2b(void)
{
    s32 digits;
    s32 value;
    s32 entry;
    u32 descriptor;
    u8* operands;
    u32 remaining;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &entry);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &value);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &digits);
    if (digits == 0)
    {
        remaining = value;
        digits = 1;
        for (;;)
        {
            remaining /= 10;
            if (remaining == 0)
            {
                break;
            }
            digits++;
        }
    }
    func_800675C8((u16)entry, value, (u8)digits);
}

/**
 * @brief Opcode 0x2C: call func_800674D8 with one operand, or with 0 through 3 when the operand has bit 7 set.
 */
void field_script_op_2c(void)
{
    s32 value;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(operands[1] & 3, operands + 2, &value);
    if (value & 0x80)
    {
        value = 0;
        do
        {
            func_800674D8((u16)value);
            value++;
        } while ((u32)value < 4);
    }
    else
    {
        func_800674D8((u16)value);
    }
}

/**
 * @brief Opcode 0x2D: read two owner-substituting operands and pass them with the active record to func_8008AFD8.
 */
void field_script_op_2d(void)
{
    s32 arg0;
    s32 arg1;
    u8 descriptor;
    u8* operands;
    FieldScriptRecord* rec;
    s32 active;
    u8* next;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    next = field_script_read_operand_or_owner(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    active = g_field_script->active_record;
    rec = FIELD_SCRIPT_RECORD(active);
    rec->pc = next;
    func_8008AFD8(arg0, arg1, rec, active);
}

/**
 * @brief Opcode 0x2E: no operation.
 */
void field_script_op_2e(void)
{
}

/**
 * @brief Opcode 0x2F: replace the low four state flags of a field record.
 * @note The first operand selects the record and substitutes the owner for 0xFF; the second is the new flag value.
 */
void field_script_op_2f(void)
{
    s32 selector;
    s32 flags;
    u8 descriptor;
    u8* operands;
    FieldScriptRecord* rec;
    s32 active;
    u8* next;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &selector);
    next = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &flags);
    active = g_field_script->active_record;
    rec = FIELD_SCRIPT_RECORD(active);
    rec->pc = next;
    func_80087614(selector, flags);
}

/**
 * @brief Opcode 0x30: queue audio sub-command 0x30 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_30(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x30);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x31: store a scaled position into the actor record matching a key.
 * @note Operands are key (owner substituted for 0xFF), x, y and z.
 */
void field_script_op_31(void)
{
    s32 key;
    s32 x;
    s32 y;
    s32 z;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &key);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &x);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &y);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &z);
    func_80087D8C(key, x, y, z);
}

/**
 * @brief Opcode 0x32: read four operands, resolve them through func_800B2654 and pass them to func_8009C620.
 * @note A first operand of 0xFF is replaced by the owner id before resolution.
 */
void field_script_op_32(void)
{
    s32 arg3;
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;
    s32 target;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg3);
    target = arg0;
    if (target == 0xFF)
    {
        target = g_field_script->status.owner_id;
    }
    arg0 = target;
    func_800B2654(&arg0, &arg1, &arg2, &arg3);
    func_8009C620(arg1, arg3, arg0, arg2);
}

/**
 * @brief Opcode 0x33: read three operands and pass them to func_8009C77C.
 * @note A first operand of 0xFF is replaced by bits 8-9 of the D_80122B78 word at 0x41C.
 */
void field_script_op_33(void)
{
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;
    s32 slot;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    if (arg0 == 0xFF)
    {
        slot = *(u32*)(D_80122B78 + 0x41C) >> 8;
        slot &= 3;
    }
    else
    {
        slot = arg0;
    }
    arg0 = slot;
    func_8009C77C(slot, arg1, arg2);
}

/**
 * @brief Opcode 0x34: play a field sound effect.
 * @note Operand types are taken from the low descriptor bits upward: sound id, pan, then an unused value.
 */
void field_script_op_34(void)
{
    s32 unused;
    s32 pan;
    s32 sound_id;
    u32 descriptor;
    u8* operands;
    FieldScriptRecord* rec;
    u8* next;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), operands + 2, &sound_id);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &pan);
    next = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &unused);
    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    rec->pc = next;
    func_800A3988(sound_id, pan, unused, rec);
}

/**
 * @brief Opcode 0x35: read two owner-substituting operands and one plain operand, then call func_8008B5D0.
 * @note Operand types are taken from the low descriptor bits upward.
 */
void field_script_op_35(void)
{
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_3(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    func_8008B5D0(arg0, arg2, 1, &arg1);
}

/**
 * @brief Opcode 0x36: queue audio sub-command 0x36 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_36(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x36);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}
