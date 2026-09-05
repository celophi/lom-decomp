#include "common.h"

/** @brief Active-sequence record; base holds unk0/unk4, records stride 0xC. */
typedef struct
{
    u8 unk0;    /* 0x00 sequence id */
    u8 pad1[3];
    s32 unk4;   /* 0x04 current record index */
    s32 unk8;   /* 0x08 record payload */
    u8 padC[4];
    s32 unk10;  /* 0x10 per-record flags */
} SeqRec;

extern u8 *g_field_script;
extern s32 D_80123FC0;

/**
 * @brief Advance the sequence cursor and stage a new timed record.
 *
 * If the current record already has a payload, advances the cursor @c unk4.
 * Writes @c D_80123FC0 + (arg0 low 16 bits) into the (possibly advanced)
 * record's @c unk8, clears then re-masks its @c unk10 low bit, and finally
 * dispatches func_800BD434 with the base @c unk0 id and notifies field_script_run.
 *
 * @param arg0 Duration/parameter; the low 16 bits are added to @c D_80123FC0.
 * @see decomp.me (100%) TODO
 */
void func_800BF2F0(s32 arg0)
{
    s32 temp_v1;
    u8 *p;

    temp_v1 = ((SeqRec *)g_field_script)->unk4;
    if (((SeqRec *)(g_field_script + (temp_v1 * 3 << 2)))->unk8 != 0)
    {
        ((SeqRec *)g_field_script)->unk4 = temp_v1 + 1;
    }
    p = g_field_script;
    ((SeqRec *)(p + (((SeqRec *)p)->unk4 * 3 << 2)))->unk8 = D_80123FC0 + (arg0 & 0xFFFF);
    ((SeqRec *)(p + (((SeqRec *)p)->unk4 * 3 << 2)))->unk10 &= ~1;
    ((SeqRec *)(p + (((SeqRec *)p)->unk4 * 3 << 2)))->unk10 &= 1;
    func_800BD434(((SeqRec *)p)->unk0, 0xD0000000, 0);
    field_script_run(g_field_script);
}
