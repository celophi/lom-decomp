#include "common.h"

/** @brief Active-sequence record; base holds unk0/unk4, records stride 0xC. */
typedef struct
{
    s32 unk0;   /* 0x00 flag word (bit 31 is the active marker) */
    s32 unk4;   /* 0x04 current record index */
    s32 unk8;   /* 0x08 */
    u8 padC[4];
    s32 unk10;  /* 0x10 per-record flags */
} SeqRec;

extern u8 *g_field_script;

/**
 * @brief Step the active sequence cursor back one record, or reset at the head.
 *
 * When the cursor @c unk4 is positive: clears the active marker bit of @c unk0
 * unless the target record's @c unk10 low bit is set, then decrements the
 * cursor. When the cursor is already at 0 or below: clears the active marker
 * and zeroes the current record's @c unk8.
 *
 * @see decomp.me (100%) TODO
 */
void func_800B85CC(void)
{
    s32 temp_v1;

    temp_v1 = ((SeqRec *)g_field_script)->unk4;
    if (temp_v1 > 0)
    {
        if ((((SeqRec *)(g_field_script + (temp_v1 * 3 << 2)))->unk10 & 1) == 0)
        {
            ((SeqRec *)g_field_script)->unk0 &= 0x7FFFFFFF;
        }
        ((SeqRec *)g_field_script)->unk4 = ((SeqRec *)g_field_script)->unk4 - 1;
        return;
    }
    ((SeqRec *)g_field_script)->unk0 &= 0x7FFFFFFF;
    ((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8 = 0;
}
