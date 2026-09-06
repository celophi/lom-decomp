#include "common.h"

/** @brief Script sequence record: 12-byte entries indexed by the header's unk4. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 *unk8;
} SeqRec;

extern u8 *volatile g_field_script;

u8 *field_script_read_operand(s32 mode, u8 *pc, s32 *out);
void field_script_branch(s32 arg0);

/**
 * @brief Script op: skip forward to the next opcode equal to the operand (or 0xFF).
 *
 * Reads the operand following the current opcode, then advances the active
 * sequence record's pc in 3-byte steps until it lands on an opcode matching
 * either the operand value or the 0xFF terminator, then takes a branch.
 */
void func_800B8B80(void)
{
    volatile s32 value;
    u8 end_op;
    u8 *pc;
    u8 *initial_next;
    u8 op;
    SeqRec *rec;

    {
        u8 *base;
        s32 index;
        base = g_field_script;
        index = ((SeqRec *)base)->unk4;
        pc = ((SeqRec *)(base + (index * 3 << 2)))->unk8;
    }
    initial_next = field_script_read_operand(pc[1] & 3, pc + 2, (s32 *)&value);
    end_op = 0xFF;
    {
        u8 *base;
        s32 index;
        SeqRec *current;
        base = g_field_script;
        index = ((SeqRec *)base)->unk4;
        current = (SeqRec *)(base + (index * 3 << 2));
        current->unk8 = initial_next;
    }
    while (1)
    {
        {
            u8 *base;
            s32 index;
            base = g_field_script;
            index = ((SeqRec *)base)->unk4;
            rec = (SeqRec *)(base + (index * 3 << 2));
        }
        pc = rec->unk8;
        op = *pc;
        if (op == end_op || op == value)
        {
            break;
        }
        rec->unk8 = pc + 3;
    }
    field_script_branch(1);
}
