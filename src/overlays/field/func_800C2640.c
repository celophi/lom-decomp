#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 unk4;
    u8 pad5;
    u16 unk6;
    u16 unk8[16];
    s32 unk28;
    s32 unk2C;
    s32 unk30;
    u8 pad34[0x5C];
    s32 unk90;
} RecC1B98;

void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
RecC1B98 *func_800C1C50(s32 id);
RecC1B98 *func_800C1B60();
void func_800B28E0(s32, s32, s32);

extern u8 *D_80122B78;

/**
 * @brief Update or clear the selected field audio record.
 * @param arg0 Record identifier.
 * @param arg1 Update value, or 0xFF to clear the active flags.
 */
void func_800C2640(s32 arg0, s32 arg1)
{
    RecC1B98 *rec;
    s32 i;

    if (arg1 != 0xFF)
    {
        rec = func_800C1C50(arg0);
        if (rec == NULL)
        {
            akao_set_song_params(0x8001, 1, 1, 1);
            return;
        }
        rec->unk90 |= 0x20000000;
        rec->unk6 = *(u16 *)(D_80122B78 + 0x686);
        i = 0;
        do
        {
            rec->unk8[i] = *(u16 *)(D_80122B78 + 0x688 + i * 2);
            i++;
        } while (i < 16);
        func_800B28E0(arg0, 7, arg1 & 0xFF);
        return;
    }

    rec = func_800C1B60(arg0);
    rec->unk90 &= 0x7FFFFFFF;
    rec->unk90 &= 0xDFFFFFFF;
}
