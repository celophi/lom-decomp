#ifndef FIELD_SCRIPT_H
#define FIELD_SCRIPT_H

#include "common.h"

/*
 * Field script interpreter.
 *
 * field_script_run executes byte-coded scripts. Each step reads the opcode at
 * the active record's program counter; opcodes below 0x40 dispatch through
 * g_field_script_op_table. A handler must advance the program counter past
 * its operands. Clearing FIELD_SCRIPT_RUNNING in the context status word ends
 * the step loop for this frame.
 *
 * Records are stacked at a 12-byte stride from the context base and
 * active_record is the depth of the current one. Record 0 aliases the header,
 * so its program counter sits at offset 8. The two words after a record's
 * program counter hold its condition flags and its wait counter.
 *
 * Operands follow a descriptor byte at pc[1] that packs one two-bit type per
 * operand, high bits first: 0 reads a script variable, 1 a byte, 2 a
 * halfword, 3 a word. field_script_read_operand decodes one operand and
 * returns the advanced pointer; field_script_read_operand_or_owner also maps
 * the value 0xFF to the script owner's id.
 */

#define FIELD_SCRIPT_RUNNING 0x80000000

/* Bit 0 of a record's flags word is the result of the last comparison. */
#define FIELD_SCRIPT_COND 0x1

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
 * @brief A record together with the two words that follow its program
 *        counter: the condition flags and the wait counter.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u8* pc;
    u32 flags;
    s32 wait;
} FieldScriptRecordState;

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

#define FIELD_SCRIPT_RECORD(index) ((FieldScriptRecord*)((u8*)g_field_script + ((index) * 3 << 2)))
#define FIELD_SCRIPT_RECORD_STATE(index) ((FieldScriptRecordState*)((u8*)g_field_script + ((index) * 3 << 2)))
#define FIELD_SCRIPT_ACTIVE_RECORD() FIELD_SCRIPT_RECORD(g_field_script->active_record)
#define FIELD_SCRIPT_ACTIVE_RECORD_STATE() FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)

u8* field_script_read_operand(u32 type, u8* data, s32* value);
u8* field_script_read_operand_or_owner(u32 type, u8* data, s32* value);
u8* field_script_read_u16(u8* data, u16* value);

/* Declared without a prototype: opcode 0x0E passes the record and its depth as extra arguments. */
void field_script_branch();
s32 func_800BD3B0();
void func_800BD434(s32 owner_id, s32 var_ref, s32 value);

#endif
