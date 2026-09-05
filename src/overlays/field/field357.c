#include "common.h"

/** @brief Active-sequence record; base holds unk4, records stride 0xC. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;   /* 0x04 current record index */
    s32 unk8;   /* 0x08 running data pointer (stored as an integer) */
} SeqRec;

extern u8 *g_field_script;

void field_script_op_00(void);

/**
 * @brief Apply a signed 16-bit relative jump to the active sequence pointer.
 *
 * Reads a little-endian 16-bit delta from the current record's data pointer at
 * offset @p arg0. A non-zero delta advances @c unk8 by it (sign-extended via the
 * 0x8000 bit); a zero delta hands off to field_script_op_00 to step the cursor.
 *
 * @param arg0 Byte offset into the record's data stream holding the delta.
 * @see decomp.me (100%) TODO
 */
void field_script_branch(s32 arg0)
{
    SeqRec *rec;
    s32 unk8;
    u8 *ptr;
    s32 val;
    s32 lo;

    rec = (SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2));
    unk8 = rec->unk8;
    ptr = (u8 *)(unk8 + arg0);
    val = ptr[0] + (ptr[1] << 8);
    lo = val & 0xFFFF;
    if (lo != 0)
    {
        if (val & 0x8000)
        {
            s32 t = unk8 + 0xFFFF0000;
            rec->unk8 = t + lo;
            return;
        }
        rec->unk8 = unk8 + lo;
        return;
    }
    field_script_op_00();
}
