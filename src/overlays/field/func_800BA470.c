#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80122B74;
extern u8 *D_80123FB8;

void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
void func_8006AB38(s32 arg0);

#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))

void func_800BA470(void)
{
    s32 state;

    state = ((u8 *)CURRENT_SEQ_REC->unk8)[1];
    if (state == 0)
    {
        D_80122B74[0x840] = 0;
        *(s32 *)(D_80122B74 + 0x858) |= 0x7F;
        func_800BD520(0, (D_80122B74[0x859] << 3) + 0xF87, 0);
        func_800BD520(0, 0x2F08, 0xFF);
    }
    else
    {
        D_80122B74[0xA90] = 0;
        *(s32 *)(D_80122B74 + 0xAA8) |= 0x7F;
        func_800BD520(0, 0x2F00, 0xFF);
    }
    func_8006AB38(state);
    CURRENT_SEQ_REC->unk8 += 2;
}
