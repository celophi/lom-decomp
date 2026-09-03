#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD204(s32 arg0, void *arg1, s32 *arg2);
void func_800674D8(s32 arg0);

#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))

/**
 * @brief Decodes a sequence argument and dispatches one or four field operations.
 *
 * Advances the current sequence record through one encoded argument. If bit 7
 * of the decoded value is set, dispatches values 0 through 3; otherwise
 * dispatches the decoded halfword directly.
 */
void func_800BAA3C(void)
{
    s32 sp10;
    u8 *temp_a1;

    temp_a1 = (u8 *)CURRENT_SEQ_REC->unk8;
    CURRENT_SEQ_REC->unk8 = func_800BD204(temp_a1[1] & 3, temp_a1 + 2, &sp10);
    if (sp10 & 0x80)
    {
        sp10 = 0;
        do
        {
            func_800674D8((u16)sp10);
            sp10++;
        } while ((u32)sp10 < 4);
    }
    else
    {
        func_800674D8((u16)sp10);
    }
}
